//\! pls update and/or find other solution...
Noggit SDL 1.4 changelog

Done:

- Add app Icon
- You can now load BGs

- Rework texture pallet
- Add load all tileset function to texture pallet
- Change rows and cols direction and count
- Add big preview of selected texture
- Discard the old current texture window
- Fix texture pallet size to fit names

- Do Rel with debug now - test do this realy work. !!!!!!

- New keys and mouse functions
- ALT + 1 till 5 set now the texture paint opercety to 100,75,50,25 and 0 %
- Space + Mouse = Speed and Pressure

- During load noggit test the config file and report common problems into the logfile. So perhaps users can fix this problems alone in the future.
- Link to Manual in modcraft  wiki
- Load MPQs now in read only. This prevent noggit fron not running if folder rights or read only dont fit on wow folder.
- Better wmo culling that the moldes dont hide all the time.
- Add ground flatten/blur speed.
- UID Fix. On save of an adt all UIDs get recalced now and saged in this and all surrounding adts.
- Clean out test and deprecated menu functions

- Auto size,tile and rotation works now also if you have copy model size and rotation activated.
- In Holes mode you can now alos edit the full chunk if you hold down the ALT key during edit.
- Water save
- Water fix for custom added water
- better Vertics Rendering ( Hanfer )
- After you have saved the selection works not. Fixed.

ToDo:

- Maptile display on minimap << Works not in the manu now because it was jsut cleard out and not fixed.
- Water Functions Assist menu actions and basic edit UI ( Steff ) 
- Add the ALT Key to texture delete from chunk function. You have to hold all 3 keys. This often cause problems of unwanted deleting in past
 

Bugs:

- Hole lines. Ground editing msut set the same values for vertics an ADT borders on both adts. ELse you get a smle line on the boarder where oyu cna look trough the ground.
- Resize bug. After the load of a map and the resice of the window the selection do not work anymore.
- Some WMOs crash noggit if renderering is turned on. Perhaps some null pointer. Like in Mulgot near start area.
- If you make a model copy to clipboard, deletethe source model and then paste it, noggit crash. On delete check clipboard and free it before.
- Some models render not or wrong. Perhaps 2 side force or normals. Some mushrooms for example in front of the cave in whispering woods.
- Fish model render wrong. Spikes all over the screen.
- Bigger models (Like stormwind) should mark and load all adts they are located on after insert, move or delete.
- There is no unloading of ADTs. We should think abut and implement
- WMO culling dont work 100%. On big models you have often parts that disapeare.
- Test again that the test that an model is on an ADT works. Some people said that some models dont work. Boundingbox test.
- During the edit (move , insert, delete) of big models (wmos) test if they mark all adts as TO SAVE. Example delete stormwind It will be there after next reload.
- Alpha layer destroy during ground editing. If you edit ground the alpha layer sames to get corrupted. You get artifacts and lines on chunk borders.


Discuss:

- U mode usage or rework to fit more. Perhaps add here also basic alpha map editing and view.

Future:

- Auto terrain painter/generator
- DBC Save function
- If DBC save Light/skybox editor. Icon is already in. Shold show then light centers with a model and perhaps sice as circle on ground if possible.
- Add texture set function. You must select 4 textures and the full adt get cleard with this. Should replace clear texture assist
- Only one selection for all. In the moment holes and all other use different selection types. THe old selection also produce holse somewhere on the map
