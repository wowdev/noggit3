# LICENSE #
This software is open source software licensed under GPL3, as found in
the COPYING file.

# DISCORD #
You can follow Noggit's development and get the latest build here: https://discord.gg/UbdFHyM

# CONTRIBUTORS #
A list of known contributors can be found in the CONTRIBUTORS
file. New maintainers should list themselves there as it is not
automatically updated.

# BUILDING #
This project requires CMake to be built. It also requires the
following libraries:

* OpenGL
* storm (stormlib by Ladislav Zezula)
* Boost
* Qt 5

Further following libraries are required for MySQL GUID Storage builds:

* LibMySQL
* MySQLCPPConn

## Windows ##
Text in `<brackets>` below are up to your choice but shall be replaced
with the same choice every time the same text is contained.

### MSVC++ ###
Any recent version of Microsoft Visual C++ should work. Be sure to
remember which version you chose as later on you will have to pick
corresponding versions for other dependencies.

### CMake ###
Any recent CMake 3.x version should work. Just take the latest.

### Boost ###
Install boost to `<boost-install>`. The easiest is to download a pre-built
package from https://sourceforge.net/projects/boost/files/boost-binaries/.

* Any version from the last years should work.
* Be sure to pick the right compiler version!
* CMake may not support the latest version yet, if you have bad timing.

### Qt5 ###
Install Qt5 to `<Qt-install>`, downloading a pre-built package from
https://www.qt.io/download-open-source/#section-2.

Note that during installation you only need **one** version of Qt and
also only **one** compiler version. If download size is noticably large
(more than a few hundred MB), you're probably downloading way too much.

### StormLib ###
Download stormlib from https://github.com/ladislav-zezula/StormLib (any
recent version).

* open CMake GUI
* set `CMAKE_INSTALL_PREFIX` (path) to `<Stormlib-install>` (folder should
  not yet exist). No other things should need to be configured.
* open solution with visual studio
* build ALL_BUILD
* build INSTALL
* Repeat for both release and debug.

### Noggit ###
* open CMake GUI
* set `CMAKE_PREFIX_PATH` (path) to `"<Qt-install>;<Stormlib-install>"`,
  e.g. `"C:/Qt/5.6/msvc2015;D:/StormLib/install"`
* set `BOOST_ROOT` (path) to `<boost-install>`, e.g. `"C:/local/boost_1_60_0"`
* (**unlikely to be required:**) move the libraries of Boost from where
  they are into `BOOST_ROOT/lib` so that CMake finds them automatically or
  set `BOOST_LIBRARYDIR` to where your lib are (.dll and .lib). Again, this
  is **highly** unlikely to be required.
* set `CMAKE_INSTALL_PREFIX` (path) to an empty destination, e.g. 
  `"C:/Users/blurb/Documents/noggitinstall`
* configure, generate
* open solution with visual studio
* build ALL_BUILD
* build INSTALL
 
To launch noggit you will need the following DLLs from Qt loadable. Install
them in the system, or copy them from `C:/Qt/X.X/msvcXXXX/bin` into the
directory containing noggit.exe, i.e. `CMAKE_INSTALL_PREFIX` configured.

* release: Qt5Core, Qt5OpenGL, Qt5Widgets, Qt5Gui
* debug: Qt5Cored, Qt5OpenGLd, Qt5Widgetsd, Qt5Guid 

## Linux ##
On **Ubuntu** you can install the building requirements using:

```bash
sudo apt install freeglut3-dev libboost-all-dev qt5-default libstorm-dev
```

Compile and build using:

```bash
mkdir build
cd build
cmake ..
make -j $(nproc)
```

Instead of `make -j $(nproc)` you may want to pick a bigger number than
`$(nproc)`, e.g. the number of `CPU cores * 1.5`.

If the build pass correctly without errors, you can go into build/bin/
and run noggit. Note that `make install` will probably work but is not
tested, and nobody has built distributable packages in years.

# DEVELOPMENT #
Feel free to ask the owner of the official repository
(https://bitbucket.org/berndloerwald/noggit3/) for write access or
fork and post a pull request.

There is a bug tracker at https://bitbucket.org/berndloerwald/noggit3/issues
which should be used.

Discord, as linked above, may be used for communication.

# CODING GUIDELINES #
Following is an example for file src/noggit/ui/foo_ban.h. .cpp files
are similar.

    // This file is part of Noggit3, licensed under GNU General Public License (version 3).
 
    //! \note Include guard shall be using #pragma once
    #pragma once
 
    //! \note Use fully qualified paths rather than "../relative". Order
    //! includes with own first, then external dependencies, then c++ STL.
    #include <noggit/bar.h>
 
    //! \note Namespaces equal directories. (java style packages.)
    namespace noggit
    {
      namespace ui
      {
        //! \note Lower case, underscore separated. Classes might have
        //! a _type suffix (even though being against the standard), but 
        //! shouldn't.
        class foo_ban : public QWidget
        {
        Q_OBJECT;
 
        public:
          //! \note Long parameter list. Would be more than 80 chars.
          //! chars. Break with comma in front. Use spaces to be
          //! aligned below the braces.
          foo_ban ( type const& name
                  , type_2 const& name_2
                  , type const& name3
                  )
            : QWidget (nullptr)
          //! \note Prefer initialization lists over assignment.
            , _var (name)
          {}
 
          //! \note Use const where possible. No space between name and
          //! braces when no arguments are given.
          void render() const;
 
          //! \note If you really need getters and setters, your design
          //! might be broken. Please avoid them or consider just a
          //! public data member.
          type const& var() const
          {
            return _var;
          }
          void var (type const& var_)
          {
            _var = var_;
          }
 
          //! \note Prefer const where possible. If you need to copy, 
          //! just take a `T`. Otherwise prefer taking a `T const&`.
          //! Don't bother when it comes to tiny types, don't bother.
          bazs_type count_some_numbers ( std::size_t begin
                                       , std::size_t end
                                       ) const
          {
            bazs_type bazs;
 
            //! \note Prefer construction over assignment. Prefer
            //! preincrement.
            for (size_t it (begin); it < end; ++it)
            {
              bazs.emplace_back (it);
            }
 
            //! \note Prefer STL algorithms over hand written code.
            auto const smallest
              (std::min_element (bazs.begin(), bazs.end()));
 
            return *smallest;
          }
 
        private:
          //! \note Member variables are prefixed with an underscore.
          type _var;
          //! \note Typedef when using complex types. Fully qualify
          //! types.
          using baz_type = type_2;
          using bazs_type = std::vector<baz_type>;
          bazs_type _bazs;
        }
      }
    }
