// This file is part of Noggit3, licensed under GNU General Public License (version 3).

// We must import noggit to use any of the API functions
require noggit

// Changes the displayed name of this script
[export]
def title(): string
	return "Hello World"


// This event fires whenever a user SELECTS this script
// This should be used to add script parameters.
[export]
def select()
    add_bool_param("Bool",false)
    add_string_param("String","Value")

// This event fires when the user left-clicks the world
[export]
def left_click()
    // We can now access parameters by its display name
    var bool_value = get_bool_param("Bool");
    var string_value = get_string_param("String");
    print("Hello World: {bool_value} {string_value}")