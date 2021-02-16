#include <QtWidgets/QFormLayout>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QCheckBox>
#include <cmath>

#include "boost/filesystem.hpp"
#include <noggit/scripting/script_vec.hpp>

#include <noggit/Log.h>
#include <noggit/scripting/scripting_tool.hpp>
#include <noggit/scripting/script_loader.hpp>
#include <noggit/tool_enums.hpp>
#include <noggit/World.h>
#include <noggit/scripting/script_context.hpp>

using namespace noggit::scripting;

static scripting_tool* cur_tool = nullptr;
scripting_tool* noggit::scripting::get_cur_tool()
{
    return cur_tool;
}

void noggit::scripting::scripting_tool::doReload()
{
    auto old_tool = cur_tool;
    cur_tool = this;
    int selection;
    try {
        selection = load_scripts();
    } catch(std::exception e)
    {
        addLog("[error]: "+std::string(e.what()));
        return;
    }
    this->script_selection->clear();
    for (auto &str : *get_scripts())
    {
        this->script_selection->addItem(str.c_str());
    }

    if (selection != -1)
    {
        this->script_selection->setCurrentIndex(selection);
    }
    cur_tool = old_tool;
}

noggit::scripting::scripting_tool::scripting_tool(QWidget *parent) : QWidget(parent)
{
    auto layout(new QFormLayout(this));
    script_selection = new QComboBox();
    layout->addRow(script_selection);

    QPushButton *btn = new QPushButton("Reload Scripts", this);
    layout->addRow(btn);
    connect(btn, &QPushButton::released, this, [this]() {
        auto old_tool = cur_tool;
        cur_tool = this;
        doReload();
        cur_tool = old_tool;
    });

    connect(script_selection, QOverload<int>::of(&QComboBox::activated), this, [this](auto index) {
        auto old_tool = cur_tool;
        cur_tool = this;

        try{
            select_script(index);
        } catch(std::exception& e)
        {
            addLog("[error]: "+std::string(e.what()));
        }
        cur_tool = old_tool;
    });

    _radius_spin = new QDoubleSpinBox(this);
    _radius_spin->setRange(0.0f, 1000.0f);
    _radius_spin->setDecimals(2);
    _radius_spin->setValue(_radius);

    _radius_slider = new QSlider(Qt::Orientation::Horizontal, this);
    _radius_slider->setRange(0, 1000);
    _radius_slider->setSliderPosition((int)std::round(_radius));

    _inner_radius_spin = new QDoubleSpinBox(this);
    _inner_radius_spin->setRange(0.0f, 1.0f);
    _inner_radius_spin->setDecimals(2);
    _inner_radius_spin->setValue(_inner_radius);
    _inner_radius_spin->setSingleStep(0.05f);

    _inner_radius_slider = new QSlider(Qt::Orientation::Horizontal, this);
    _inner_radius_slider->setRange(0, 100);
    _inner_radius_slider->setSliderPosition((int)std::round(_inner_radius * 100));

    QGroupBox *radius_group(new QGroupBox("Radius"));
    QFormLayout *radius_layout(new QFormLayout(radius_group));
    radius_layout->addRow("Outer", _radius_spin);
    radius_layout->addRow("Outer:", _radius_spin);
    radius_layout->addRow(_radius_slider);
    radius_layout->addRow("Inner:", _inner_radius_spin);
    radius_layout->addRow(_inner_radius_slider);

    layout->addWidget(radius_group);

    _script_settings_group = new QGroupBox("Script Settings");
    _script_settings_layout = new QFormLayout(_script_settings_group);
    layout->addWidget(_script_settings_group);

    _description = new QLabel(this);
    layout->addWidget(_description);

    _log = new QPlainTextEdit(this);
    _log->setReadOnly(true);
    layout->addWidget(_log);

    connect(_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
        _radius = v;
        QSignalBlocker const blocker(_radius_slider);
        _radius_slider->setSliderPosition((int)std::round(v));
    });

    connect(_radius_slider, &QSlider::valueChanged, [&](int v) {
        _radius = v;
        QSignalBlocker const blocker(_radius_spin);
        _radius_spin->setValue(v);
    });

    connect(_inner_radius_spin, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double v) {
        _inner_radius = v;
        QSignalBlocker const blocker(_inner_radius_slider);
        _inner_radius_slider->setSliderPosition((int)std::round(v * 100));
    });

    connect(_inner_radius_slider, &QSlider::valueChanged, [&](int v) {
        _inner_radius = v / 100.0f;
        QSignalBlocker const blocker(_inner_radius_spin);
        _inner_radius_spin->setValue(_inner_radius);
    });
    doReload();
}

#define ADD_SLIDER(T,min,max,def,decimals,holder) \
    double dp1 = decimals > 0 ? decimals + 5 : decimals + 1; \
    auto spinner = new QDoubleSpinBox(this); \
    spinner->setRange(min, max); \
    spinner->setDecimals(decimals); \
    spinner->setValue(def); \
    auto slider = new QSlider(Qt::Orientation::Horizontal, this); \
    slider->setRange(min * dp1, max * dp1); \
    slider->setSliderPosition((int)std::round(def * dp1)); \
    auto label = new QLabel(this); \
    label->setText(name.c_str()); \
    connect(spinner, qOverload<double>(&QDoubleSpinBox::valueChanged), [=](double v) { \
        holder->set((T)v); \
        QSignalBlocker const blocker(slider); \
        slider->setSliderPosition((int)std::round(v * dp1)); \
    }); \
    connect(slider, &QSlider::valueChanged, [=](int v) { \
        double t = double(v) / dp1; \
        holder->set((T)t); \
        QSignalBlocker const blocker(spinner); \
        spinner->setValue(t); \
    }); \
    \
    holder->set(def); \
    _script_settings_layout->addRow(label); \
    _script_settings_layout->addRow(spinner); \
    _script_settings_layout->addRow(slider); \
    _script_widgets.push_back(label); \
    _script_widgets.push_back(spinner); \
    _script_widgets.push_back(slider); \
    _holders.push_back(holder); 

double_holder *noggit::scripting::scripting_tool::addDouble(std::string name, double min, double max, double def)
{
    auto holder = new double_holder();
    ADD_SLIDER(double,min, max, def, 2, holder);
    return holder;
}

int_holder *noggit::scripting::scripting_tool::addInt(std::string name, int min, int max, int def)
{
    auto holder = new int_holder();
    ADD_SLIDER(int, min, max, def, 0, holder);
    return holder;
}

bool_holder *noggit::scripting::scripting_tool::addBool(std::string name, bool def)
{
    auto holder = new bool_holder();
    auto checkbox = new QCheckBox(this);
    checkbox->setCheckState(Qt::CheckState::Checked);
    auto label = new QLabel(this);
    label->setText(name.c_str());
    holder->set(def);
    _holders.push_back(holder);
    connect(checkbox, &QCheckBox::stateChanged, this, [=](auto value) {
        holder->set(value ? true : false);
    });

    _script_widgets.push_back(checkbox);
    _script_widgets.push_back(label);
    _script_settings_layout->addRow(label);
    _script_settings_layout->addRow(checkbox);
    return holder;
}

string_holder *noggit::scripting::scripting_tool::addString(std::string name, std::string def)
{
    auto tline = new QLineEdit(this);
    tline->setPlaceholderText(def.c_str());
    auto label = new QLabel(this);
    label->setText(name.c_str());
    auto h = new string_holder();
    h->set(def);
    connect(tline, &QLineEdit::textChanged, this, [=](auto text) {
        h->set(text.toUtf8().constData());
    });

    _script_widgets.push_back(label);
    _script_widgets.push_back(tline);
    _script_settings_layout->addRow(label);
    _script_settings_layout->addRow(tline);
    _holders.push_back(h);
    return h;
}

void noggit::scripting::scripting_tool::removeScriptWidgets()
{
    for (auto &widget : _script_widgets)
    {
        _script_settings_layout->removeWidget(widget);
        delete widget;
    }

    for (auto &holder : _holders)
    {
        delete holder;
    }

    _holders.clear();
    _script_widgets.clear();
}

void noggit::scripting::scripting_tool::sendUpdate(
    World *world,
    math::vector_3d pos,
    noggit::camera *cam,
    float dt,
    bool left_mouse,
    bool right_mouse,
    bool holding_shift,
    bool holding_ctrl,
    bool holding_alt,
    bool holding_space)
{
    auto old_tool = cur_tool;
    cur_tool = this;
    script_context ctx(world, pos, brushRadius(), innerRadius(), cam, holding_alt, holding_shift, holding_ctrl, holding_space);

    try {
        if (left_mouse)
        {
            if (!_last_left)
            {
                send_left_click(&ctx);
            }

            if(holding_shift)
            {
                auto h = ctx.select(ctx.pos()->x(),ctx.pos()->z(),100,100);
                h->chunk_add_texture("tileset/durotar/durotardirt.blp");
                h->chunk_add_texture("tileset/durotar/durotargrass.blp");

                while(h->is_on_chunk())
                {
                    while(h->is_on_vertex())
                    {
                        h->vert_set_y(20);
                        while(h->is_on_tex())
                        {
                            h->tex_set_alpha(1,0.3);
                            h->next_tex();
                        }
                        h->next_vertex();
                    }
                    h->chunk_apply_all();
                    h->next_chunk();
                }
            }
            else
            {
                send_left_hold(&ctx);
            }
        }

        if (right_mouse)
        {
            if (!_last_right)
            {
                send_right_click(&ctx);
            }
            send_right_hold(&ctx);

        }

        if (!left_mouse && _last_left)
        {
            send_left_release(&ctx);
        }

        if (!right_mouse && _last_right)
        {
            send_right_release(&ctx);
        }

    } catch(const std::exception &e) {
        addLog(("[error]: "+std::string(e.what())));
    }

    _last_left = left_mouse;
    _last_right = right_mouse;
    cur_tool = old_tool;
}

void noggit::scripting::scripting_tool::addDescription(std::string text)
{
    _description->setText(_description->text() + "\n" + text.c_str());
}

void noggit::scripting::scripting_tool::addLog(std::string text)
{
    LogDebug << "[script window]: " << text << "\n";
    _log->appendPlainText(text.c_str());
    _log->verticalScrollBar()->setValue(_log->verticalScrollBar()->maximum());
}

void noggit::scripting::scripting_tool::clearLog()
{
    _log->setPlainText("");
}

void noggit::scripting::scripting_tool::clearDescription()
{
    _description->setText("");
}