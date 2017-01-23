# MapIndex::fixUIDs() concept idea

## Step 1:
* read every tile
* for each tile read all the modf and mddf entries
* discard the entries not placed on the current tile (eg entry.pos isn't inside the tile)
* discard duplicates
* read the filenames
* create the model/wmo instances and add them to a list


## Step 2:
* create 2 lists for each tile, one for the wmo and one for the m2
* assign a new uid for the instances (starting from 0)
* add every model/wmo instances to the lists of every tiles they are on


## Step 3:
* load each tile without the models/wmos
* save the tile with the model/wmo instances from step 2


## Step 4:
* save the highest uid on the disc to not have to repeat the process ever again

