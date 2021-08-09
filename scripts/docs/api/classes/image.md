# Class: image

Represents a bitmap image

images can be created from create_image or load_png

## Table of contents

### Constructors

- [constructor](image.md#constructor)

### Methods

- [get\_alpha](image.md#get_alpha)
- [get\_blue](image.md#get_blue)
- [get\_green](image.md#get_green)
- [get\_pixel](image.md#get_pixel)
- [get\_red](image.md#get_red)
- [gradient\_scale](image.md#gradient_scale)
- [height](image.md#height)
- [save](image.md#save)
- [set\_pixel](image.md#set_pixel)
- [set\_pixel\_floats](image.md#set_pixel_floats)
- [width](image.md#width)

## Constructors

### constructor

\+ **new image**(): [*image*](image.md)

**Returns:** [*image*](image.md)

## Methods

### get\_alpha

▸ **get_alpha**(`x`: *number*, `y`: *number*): *number*

Returns the alpha channel (between 0-1) at an image coordinate

#### Parameters:

Name | Type |
:------ | :------ |
`x` | *number* |
`y` | *number* |

**Returns:** *number*

___

### get\_blue

▸ **get_blue**(`x`: *number*, `y`: *number*): *number*

Returns the blue channel (between 0-1) at an image coordinate

#### Parameters:

Name | Type |
:------ | :------ |
`x` | *number* |
`y` | *number* |

**Returns:** *number*

___

### get\_green

▸ **get_green**(`x`: *number*, `y`: *number*): *number*

Returns the green channel (between 0-1) at an image coordinate

#### Parameters:

Name | Type |
:------ | :------ |
`x` | *number* |
`y` | *number* |

**Returns:** *number*

___

### get\_pixel

▸ **get_pixel**(`x`: *number*, `y`: *number*): *number*

Returns the pixel value at a coordinate

#### Parameters:

Name | Type |
:------ | :------ |
`x` | *number* |
`y` | *number* |

**Returns:** *number*

___

### get\_red

▸ **get_red**(`x`: *number*, `y`: *number*): *number*

Returns the red channel (between 0-1) at an image coordinate

#### Parameters:

Name | Type |
:------ | :------ |
`x` | *number* |
`y` | *number* |

**Returns:** *number*

___

### gradient\_scale

▸ **gradient_scale**(`rel`: *number*): *number*

Returns the pixel value at a relative horizontal coordinate

#### Parameters:

Name | Type | Description |
:------ | :------ | :------ |
`rel` | *number* | horizontal relative position (between 0-1)    |

**Returns:** *number*

___

### height

▸ **height**(): *number*

Returns the height of this image

**Returns:** *number*

___

### save

▸ **save**(`filename`: *string*): *any*

Saves this image to a file

#### Parameters:

Name | Type |
:------ | :------ |
`filename` | *string* |

**Returns:** *any*

___

### set\_pixel

▸ **set_pixel**(`x`: *number*, `y`: *number*, `value`: *number*): *void*

Sets the pixel value at an image coordinate

#### Parameters:

Name | Type |
:------ | :------ |
`x` | *number* |
`y` | *number* |
`value` | *number* |

**Returns:** *void*

___

### set\_pixel\_floats

▸ **set_pixel_floats**(`x`: *number*, `y`: *number*, `r`: *number*, `g`: *number*, `b`: *number*, `a`: *number*): *void*

Sets the pixel value at an image coordinate

#### Parameters:

Name | Type | Description |
:------ | :------ | :------ |
`x` | *number* |  |
`y` | *number* |  |
`r` | *number* | should be between 0-1   |
`g` | *number* | should be between 0-1   |
`b` | *number* | should be between 0-1   |
`a` | *number* | should be between 0-1    |

**Returns:** *void*

___

### width

▸ **width**(): *number*

Returns the width of this image

**Returns:** *number*
