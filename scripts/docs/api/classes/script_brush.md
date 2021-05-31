# Class: script\_brush

Represents a script brush in the script window.

## Table of contents

### Constructors

- [constructor](script_brush.md#constructor)

### Properties

- [on\_left\_click](script_brush.md#on_left_click)
- [on\_left\_hold](script_brush.md#on_left_hold)
- [on\_left\_release](script_brush.md#on_left_release)
- [on\_right\_click](script_brush.md#on_right_click)
- [on\_right\_hold](script_brush.md#on_right_hold)
- [on\_right\_release](script_brush.md#on_right_release)

### Methods

- [add\_bool\_tag](script_brush.md#add_bool_tag)
- [add\_description](script_brush.md#add_description)
- [add\_int\_tag](script_brush.md#add_int_tag)
- [add\_null\_tag](script_brush.md#add_null_tag)
- [add\_real\_tag](script_brush.md#add_real_tag)
- [add\_string\_list\_tag](script_brush.md#add_string_list_tag)
- [add\_string\_tag](script_brush.md#add_string_tag)
- [get\_name](script_brush.md#get_name)
- [set\_name](script_brush.md#set_name)

## Constructors

### constructor

\+ **new script_brush**(): [*script\_brush*](script_brush.md)

**Returns:** [*script\_brush*](script_brush.md)

## Properties

### on\_left\_click

• **on\_left\_click**: [*callback*](../modules.md#callback)<(`brush`: [*script\_brush*](script_brush.md), `event`: [*script\_brush\_event*](script_brush_event.md)) => *void*\>

The function to call when the user left clicks
the world with this brush

___

### on\_left\_hold

• **on\_left\_hold**: [*callback*](../modules.md#callback)<(`brush`: [*script\_brush*](script_brush.md), `event`: [*script\_brush\_event*](script_brush_event.md)) => *void*\>

The funciton to call when the user holds the left mouse button
in the world with this brush

___

### on\_left\_release

• **on\_left\_release**: [*callback*](../modules.md#callback)<(`brush`: [*script\_brush*](script_brush.md), `event`: [*script\_brush\_event*](script_brush_event.md)) => *void*\>

The function to call when the user releases the left moues button
in the world with this brush

___

### on\_right\_click

• **on\_right\_click**: [*callback*](../modules.md#callback)<(`brush`: [*script\_brush*](script_brush.md), `event`: [*script\_brush\_event*](script_brush_event.md)) => *void*\>

The function to call when the user right clicks
the world with this brush

___

### on\_right\_hold

• **on\_right\_hold**: [*callback*](../modules.md#callback)<(`brush`: [*script\_brush*](script_brush.md), `event`: [*script\_brush\_event*](script_brush_event.md)) => *void*\>

The funciton to call when the user holds the right mouse button
in the world with this brush

___

### on\_right\_release

• **on\_right\_release**: [*callback*](../modules.md#callback)<(`brush`: [*script\_brush*](script_brush.md), `event`: [*script\_brush\_event*](script_brush_event.md)) => *void*\>

The function to call when the user releases the right mouse button
in the world with this brush

## Methods

### add\_bool\_tag

▸ **add_bool_tag**(`item`: *string*, `def`: *boolean*): [*tag*](tag.md)<boolean\>

Adds a bool tag to the settings panel

#### Parameters:

Name | Type |
:------ | :------ |
`item` | *string* |
`def` | *boolean* |

**Returns:** [*tag*](tag.md)<boolean\>

___

### add\_description

▸ **add_description**(`text`: *string*): *any*

Adds a description row to the brush window

#### Parameters:

Name | Type |
:------ | :------ |
`text` | *string* |

**Returns:** *any*

___

### add\_int\_tag

▸ **add_int_tag**(`item`: *string*, `low`: *number*, `high`: *number*, `def`: *number*): [*tag*](tag.md)<number\>

Adds an integer tag to the settings panel

#### Parameters:

Name | Type |
:------ | :------ |
`item` | *string* |
`low` | *number* |
`high` | *number* |
`def` | *number* |

**Returns:** [*tag*](tag.md)<number\>

___

### add\_null\_tag

▸ **add_null_tag**(): *any*

Adds an empty tag, typically used to create empty lines in the settings panel

**Returns:** *any*

___

### add\_real\_tag

▸ **add_real_tag**(`item`: *string*, `low`: *number*, `high`: *number*, `def`: *number*): [*tag*](tag.md)<number\>

Adds a real (i.e. float) tag to the settings panel

#### Parameters:

Name | Type |
:------ | :------ |
`item` | *string* |
`low` | *number* |
`high` | *number* |
`def` | *number* |

**Returns:** [*tag*](tag.md)<number\>

___

### add\_string\_list\_tag

▸ **add_string_list_tag**(`item`: *string*, ...`values`: *string*[]): [*tag*](tag.md)<string\>

Adds a dropdown menu to the settings panel

#### Parameters:

Name | Type |
:------ | :------ |
`item` | *string* |
`...values` | *string*[] |

**Returns:** [*tag*](tag.md)<string\>

___

### add\_string\_tag

▸ **add_string_tag**(`item`: *string*, `def`: *string*): [*tag*](tag.md)<string\>

Adds a string tag to the settings panel

#### Parameters:

Name | Type |
:------ | :------ |
`item` | *string* |
`def` | *string* |

**Returns:** [*tag*](tag.md)<string\>

___

### get\_name

▸ **get_name**(): *string*

Returns the current name of this script brush

**Returns:** *string*

___

### set\_name

▸ **set_name**(`name`: *string*): *void*

Changes the name of this script brush

#### Parameters:

Name | Type |
:------ | :------ |
`name` | *string* |

**Returns:** *void*
