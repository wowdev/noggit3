# MapIndex::fixUIDs() concept idea (Adspartan)

## Step 1:
* read every tile
* for each tile read all the modf and mddf entries
* discard the entries not placed on the current tile (eg entry.pos isn't inside the tile)
* discard duplicates
* read the filenames
* create the model/wmo instances and add them to a list

-> [tile -> [(m*df, filename)]] where (m*df, filename) is unique by transformation, only entries with inside (tile, .pos) are stored

## Step 2:
* create 2 lists for each tile, one for the wmo and one for the m2
* assign a new uid for the instances (starting from 0)
* add every model/wmo instances to the lists of every tiles they are on

-> [[tile] -> [(uid, m*df, filename)] where uid is 0..n, all entries where intersect (tile, .extends) are stored

## Step 3:
* load each tile without the models/wmos
* save the tile with the model/wmo instances from step 2

## Step 4:
* save the highest uid on the disc to not have to repeat the process ever again

## Claim
* Every entity has a globally unique id in 2I, 1O
* uniqueness of newly placed objects is guaranteed by storing highest uid on disk

## Shortcomings
* does not work with parallel work by multiple editors on same map, or
  requires a central fix-pass every so often, which is undesirable
  -> requires one pass _after a fuckup_ which still can exist
  -> valid and good for single-person work
