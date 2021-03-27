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
