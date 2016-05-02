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
* GLEW
* Freetype2
* storm (stormlib by Ladislav Zezula)
* Boost
* SDL 1.2

For Windows there is a repo with all needed libs to compile noggit3.
Read the description there before you start.

https://bitbucket.org/modcraft/noggit3libs

# DEVELOPMENT #
Feel   free   to   ask   the   owner  of   the   official   repository
(https://bitbucket.org/berndloerwald/noggit3/)  for  write  access  or
fork and post a pull request.

There is a bug tracker at http://modcraft.superparanoid.de/bugtracker/index.php which should be used.

# CODING GUIDELINES #
Following is  an example for file  src/noggit/ui/foo_ban.h. .cpp files
are similar.

    // foo_ban.h is part of Noggit3, licensed via GNU General Publiicense (version 3).
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
            : QWidget (NULL)
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