# Spelunky 2 plugin for x64dbg

## Installation

- Download [x64dbg](https://github.com/x64dbg/x64dbg/releases) and extract.
- In the extracted folder, go to release/x64 and launch x64dbg.exe, this will initialize the application and create a bunch of folders.
- Close the application.
- An (empty) plugins folder has been created. Put the following files in this plugin folder:
  - Spelunky2.dp64 (the actual plugin)
  - Spelunky2.json (the definition of the fields and classes)
  - Spelunky2Entities.txt (the list of all the entities)
  - Spelunky2ParticleEmitters.txt (the list of all the particle emitters)
  - Spelunky2RoomCodes.json (the list of the room codes for level generation)
- Launch x64dbg.exe again, and you should see a tab at the top right of the window called "Spelunky 2"

By default, x64dbg enables a couple of standard/system breakpoints, which means that Spelunky will pause automatically when these breakpoints are hit. To disable these, open the Options > Preferences menu and uncheck "System Breakpoint, "Entry Breakpoint" and "TLS Callbacks".

If you do hit a breakpoint, the bottom left corner of the x64dbg window will be a yellow square with red text "Paused". Just click the "Run" icon in the toolbar at the top (the blue right-pointing arrow), to continue execution.

## Starting up

- Launch Spelunky 2, and wait until you see the Mossmouth logo, don't attach before that point, during the black screen
- In x64dbg go to the File > Attach menu and a list of processes will pop up. Choose Spel2 (Spelunky 2) and click the Attach button
- Activate the Spelunky 2 tab (the rightmost tab), and you're good to go

## Basic usage

The buttons on the top-left side give you access to the internals of Spelunky 2.

The data tables containing all the fields will have a clickable "Value" column, to either change its value, or jump to a represented entity, type, ...

Fields that are marked in red means they have changed values compared to the last refresh update.
The State and Entity windows have a refresh button, and a way to automatically refresh the data.

## Entity DB

The search bar at the top allows you to enter the numerical value of the type to look up, or you can type the name.

![EntityDB](/resources/docs_entitydb.png)

Click the 'Compare' tab and choose a field from the dropdown to see a list of all the entities and the value of the chosen field.

![EntityDB](/resources/docs_entitydb_compare.png)

Click the 'Group by value' checkbox to get a list of the unique values of the field, and which entities belong to that group.

![EntityDB](/resources/docs_entitydb_compare_grouped.png)

## Particle DB

Similar to the Entity DB, but for all the particle emitters in the game.

![ParticleDB](/resources/docs_particledb.png)

## Texture DB

Shows a list of all the textures in the game.

![TextureDB](/resources/docs_texturedb.png)

## State

![State](/resources/docs_state.png)

## Level generation

![LevelGen](/resources/docs_levelgen.png)

The 'Rooms' tab shows how the different rooms are laid out in the level during level generation. Hover over the room code to see a description.

![LevelGen](/resources/docs_levelgen_rooms.png)

## Entities

By default, the entities list is filtered only by characters, monsters and items, as things tend to get a bit slow if you always list the FLOOR entities as well.

![Entities](/resources/docs_entities.png)

The detail screen of an entity allows you to not only see the fields, but also its memory representation, and the position of the entity in the level, indicated by the magenta dot.

![Entity Fields](/resources/docs_entity_fields.png)

![Entity Memory](/resources/docs_entity_memory.png)

Hover over the field to see the name on the tooltip.

![Entity Level](/resources/docs_entity_level.png)

The C++ tab gives you a copy-pasteable C++ header for use in e.g. Overlunky.

![Entity c++ header](/resources/docs_entity_cpp.png)


You can compare two entities by first opening an entity window, and then dragging another entity from the 'Entities' window on top of the original entity window. Two more columns will appear in the fields table. The fields with an orange background differ from the original entity.

![Entity Fields Comparison](/resources/docs_entity_compare_fields.png)

The memory viewer (as well as the level viewer) also show the other entity for comparison.

![Entity Memory Comparison](/resources/docs_entity_compare_memory.png)

## Strings table

Shows a list of all the strings defined in the game.

![Strings table](/resources/docs_stringstable.png)


## Advanced usage

The Spelunky2.json file contains all the field definitions of the known classes. Just add another entry, and specify the correct field types, which you can deduce from looking at the entity memory tab. Don't forget to add the new entity name to the `entity_class_hierarchy` list so the correct inheritance can be determined, and to `default_entity_types` so that when you click on the entity, it will immediately cast it to the correct type. You can use a regex to match multiple entity names at once.

If you define a new pointer type, don't forget to add it to the `pointer_types` list.

Once saved, click the "Reload JSON" button at the bottom left, and the updated information will be visualized (the entity windows will close for reload though).

Entity, State, EntityDB and ParticleDB windows all have a "Label" button as well. This can help you if you are reading the assembly in the CPU tab. Click the "Clear labels" button to remove them, however due to a bug (?) in x64dbg it won't delete them all. Press Ctrl-Alt-L to see all the labels.

## Virtual table

![Virtual table](/resources/docs_virtual_table.png)

To look up an entity in the virtual table, make sure you are in a level containing the entity in question, and press the 'Detect entities' button. Uncheck the imported symbols and non-address entries to clear up the list. If you leave 'Show symbol-less entries' checked, you will jump to the entity when you type the symbol name in the search box. If you uncheck it, just the filtered list will be shown.

The table offset is the offset from the first entry in the table, D3Dcompile.

![Virtual table lookup](/resources/docs_virtual_table_lookup.png)

To look up the offset of a specific function relative to the base _vftable of an entry, right click somewhere in the function (in the CPU tab) and choose Spelunky2 > Lookup in virtual table. A list will be shown with all preceding named symbols, and the relative offset this function has.
