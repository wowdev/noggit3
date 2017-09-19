# LICENSE #
This software is open source software licensed under GPL3, as found in
the COPYING file.

# CONTRIBUTORS #
A  list  of  known  contributors  can be  found  in  the  CONTRIBUTORS
file.  New maintainers  should  list  themselves there  as  it is  not
automatically updated.

# BUILDING #
This  project  requires  CMake  to  be built.  It  also  requires  the
following libraries:

* OpenGL
* storm (stormlib by Ladislav Zezula)
* Boost
* Qt 5

Further following libraries are required for MySQL GUID Storage builts:

* LibMySQL
* MySQLCPPConn

## Windows ##

* install msvc++
* install cmake
* install boost via https://sourceforge.net/projects/boost/files/boost-binaries/ (be sure to use the right version and compiler version. cmake may not support the latest version) to <boost-install>
* install Qt via https://www.qt.io/download-open-source/#section-2 to <Qt-install>
* download stormlib from https://github.com/ladislav-zezula/StormLib
 * [stormlib] open CMake GUI
 * configure, generate stormlib (save the cache entry CMAKE_INSTALL_PREFIX path somewhere, set it to a empty folder if not present)
 * open solution with visual studio
 * build ALL_BUILD, then INSTALL, do it for both release and debug
* [noggit] open CMake GUI
 * set cache entry CMAKE_PREFIX_PATH (path) to <Qt-install>;<stormlib-build>, e.g. C:/Qt/5.6/msvc2015;<Stormlib's CMAKE_INSTALL_PREFIX path>
 * set cache entry BOOST_ROOT (path) to <boost-install>, e.g. C:/local/boost_1_60_0 
 * put the lib in BOOST_ROOT/lib so that cmake finds them automatically or set BOOST_LIBRARYDIR to where your lib are (.dll and .lib)
 * set cache entry CMAKE_INSTALL_PREFIX (path) to a empty destination, e.g. C:/Users/loerwald/Documents/noggitinstall
 * configure, generate
 * open solution with visual studio
 * build ALL_BUILD
 
 To launch noggit you will need the following dll from qt (Qt/X.X/msvcXXXX/bin):
  * release: Qt5Core, Qt5OpenGL, Qt5Widgets, Qt5Gui
  * debug: Qt5Cored, Qt5OpenGLd, Qt5Widgetsd, Qt5Guid 

# DEVELOPMENT #
Feel   free   to   ask   the   owner  of   the   official   repository
(https://bitbucket.org/berndloerwald/noggit3/)  for  write  access  or
fork and post a pull request.

There is a bug tracker at https://bitbucket.org/berndloerwald/noggit3/issues which should be used.

# CODING GUIDELINES #
Following is  an example for file  src/noggit/ui/foo_ban.h. .cpp files
are similar.

    // This file is part of Noggit3, licensed under GNU General Public License (version 3).
    // First Lastname <MAIL@ADDRESS>
 
    //! \note Include guard shall be the full path except for src/.
    #ifndef NOGGIT_UI_FOO_H
    #define NOGGIT_UI_FOO_H
 
    //! \note   Use  fully   qualified  paths.   Standard   >  external
    //! dependencies > own.
    #include <noggit/bar.h>
 
    //! \note Namespaces equal directories. (java style packages.)
    namespace noggit
    {
      namespace ui
      {
        //! \note Lower case,  underscore separated. Classes might have
        //! a _type suffix (even though being against the standard)
        class foo_ban : public QWidget
        {
        Q_OBJECT
 
        public:
          //! \note  Long  parameter  list.   Would  be  more  than  80
          //! chars.  Break  with comma  in  front.  Use  spaces to  be
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
          //! might be broken.
          type const& var() const
          {
            return _var;
          }
          //! \note One  might use setter chaining.  (just as operator=
          //! returns the assigned value)
          type const& var (type const& var_)
          {
            return _var = var_;
          }
 
          //! \note Prefer const (references) where possible.
          bazs_type count_some_numbers ( const size_t& begin
                                       , const size_t& end
                                       ) const
          {
            bazs_type bazs;
 
            //! \note  Prefer   construction  over  assignment.  Prefer
            //! preincrement.
            for (size_t it (begin); it < end; ++it)
            {
              bazs.push_back (it);
            }
 
            //! \note Prefer stl algorithms over hand written code.
            const bazs_type::const_iterator smallest
              (std::min_element (bazs.begin(), bazs.end()));
 
            return *smallest;
          }
 
        private:
          //! \note Member variables are prefixed with an underscore.
          type _var;
          //! \note  Typedef when  using complex  types.  Fully qualify
          //! types.
          using baz_type = type_2;
          using bazs_type = std::vector<baz_type>;
          bazs_type _bazs;
        }
      }
    }