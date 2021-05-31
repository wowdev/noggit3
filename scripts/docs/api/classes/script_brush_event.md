# Class: script\_brush\_event

Represents the event context passed to brush click events.

## Table of contents

### Constructors

- [constructor](script_brush_event.md#constructor)

### Methods

- [dt](script_brush_event.md#dt)
- [inner\_radius](script_brush_event.md#inner_radius)
- [outer\_radius](script_brush_event.md#outer_radius)
- [pos](script_brush_event.md#pos)
- [set\_inner\_radius](script_brush_event.md#set_inner_radius)
- [set\_outer\_radius](script_brush_event.md#set_outer_radius)

## Constructors

### constructor

\+ **new script_brush_event**(): [*script\_brush\_event*](script_brush_event.md)

**Returns:** [*script\_brush\_event*](script_brush_event.md)

## Methods

### dt

▸ **dt**(): *number*

Returns the delta-time since the last update frame

**Returns:** *number*

___

### inner\_radius

▸ **inner_radius**(): *number*

Returns the current inner radius configured in the settings panel

**Returns:** *number*

___

### outer\_radius

▸ **outer_radius**(): *number*

Returns the current outer radius configured in the settings panel

**Returns:** *number*

___

### pos

▸ **pos**(): [*vector\_3d*](vector_3d.md)

Returns world position of this click event.
i.e. the world position where the user clicked, held or released a mouse button

**Returns:** [*vector\_3d*](vector_3d.md)

___

### set\_inner\_radius

▸ **set_inner_radius**(`value`: *number*): *any*

Sets the outer radius in the settings panel for this brush

#### Parameters:

Name | Type | Description |
:------ | :------ | :------ |
`value` | *number* | should be between 0-1    |

**Returns:** *any*

___

### set\_outer\_radius

▸ **set_outer_radius**(`value`: *number*): *any*

Sets the outer radius in the settings panel for this brush

#### Parameters:

Name | Type |
:------ | :------ |
`value` | *number* |

**Returns:** *any*
