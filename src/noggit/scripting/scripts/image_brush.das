// Made by TSWoW (https://github.com/tswow), licensed under GNU General Public License (version 3).
require noggit

[export]
def title(): string
	return "Image Brush"

[export]
def select()
    add_string_param("Filename","")

[export]
def left_click()
    var img = create_image()
    img_load_png(img,get_string_param("Filename"))

    var origin = pos()

    var width = img_width(img);
    var height = img_height(img);

    var half_width = float(width)/2.0
    var half_height = float(height)/2.0

    var sel = make_selector()
    select_origin(sel,pos(),float(width),float(height))

    while sel_next_chunk(sel)
        var chunk = sel_get_chunk(sel)
        while chunk_next_vert(chunk)
            var vert = chunk_get_vert(chunk)
            var angle = cam_yaw()-90.0
            var pos = rotate_2d(vert_get_pos(vert),origin,angle)

            var local_x = int(round(pos.x - origin.x + half_width));
            var local_z = int(round(pos.z - origin.z + half_height));

            if(local_x < 0 || local_x >= width || local_z < 0 || local_z >= height)
                continue

            var pixel = img_get_pixel(img,local_x,local_z);
            var red = float((pixel>>24u)&0xff)/255.0
            var green = float((pixel>>16u)&0xff)/255.0
            var blue = float((pixel>>8u)&0xff)/255.0

            // Skip transparent pixels
            if (pixel&0xff)<100u
                continue

            vert_set_color(vert,red,green,blue)

        chunk_apply_all(chunk)