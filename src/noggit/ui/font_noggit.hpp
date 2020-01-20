// This file is part of Noggit3, licensed under GNU General Public License (version 3).

#pragma once

#include <noggit/ui/font_awesome.hpp>

#include <QtGui/QIcon>


namespace noggit
{
  namespace ui
  {
    struct font_noggit
    {
      enum icons
      {
        rmb = 0xf868,
        lmb = 0xf864,
        mmb = 0xf866,
        rmb_drag = 0xf869,
        lmb_drag = 0xf865,
        mmb_drag = 0xf867,
        drag = 0xf86A,
        alt = 0xf86B,
        ctrl = 0xf86C,
        shift = 0xf86D,
        a = 0xf86E,
        b = 0xf86F,
        c = 0xf870,
        d = 0xf871,
        e = 0xf872,
        f = 0xf873,
        g = 0xf874,
        h = 0xf875,
        i = 0xf876,
        j = 0xf877,
        k = 0xf878,
        l = 0xf879,
        m = 0xf87A,
        n = 0xf87B,
        o = 0xf87C,
        p = 0xf87D,
        q = 0xf87E,
        r = 0xf87F,
        s = 0xf880,
        t = 0xf881,
        u = 0xf882,
        v = 0xf883,
        w = 0xf884,
        x = 0xf885,
        y = 0xf886,
        z = 0xf887,
        space = 0xf888,
        page_up = 0xf889,
        page_down = 0xf88A,
        home = 0xf88B,
        num = 0xf88C,
        tab = 0xf88D,
        plus = 0xf88E,
        minus = 0xf88F,
        f1 = 0xf890,
        f2 = 0xf891,
        f3 = 0xf892,
        f4 = 0xf893,
        f5 = 0xf894,
        f6 = 0xf895,
        f7 = 0xf896,
        f8 = 0xf897,
        f9 = 0xf898,
        f10 = 0xf899,
        f11 = 0xf89A,
        f12 = 0xf89B     
      };
    };

    class font_noggit_icon : public QIcon
    {
    public:
      font_noggit_icon(font_noggit::icons const&);
    };

    class font_noggit_icon_engine : public font_awesome_icon_engine
    {
    public:
      font_noggit_icon_engine(const QString& text);

      virtual font_noggit_icon_engine* clone() const override;

      virtual void paint(QPainter* painter, QRect const& rect, QIcon::Mode mode, QIcon::State state) override;


    private:
      const QString _text;

      static std::map<int, QFont> _fonts;

    };

  }
}
