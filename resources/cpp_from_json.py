import re
import json
import urllib.request

print("Preparing ...")


# # Download Spelunky2.json from the x64dbg github repo
# #url = "https://gitcdn.link/repo/spelunky-fyi/Spelunky2X64DbgPlugin/master/resources/Spelunky2.json"
# url = "https://raw.githubusercontent.com/spelunky-fyi/Spelunky2X64DbgPlugin/master/resources/Spelunky2.json"
# response = urllib.request.urlopen(url)
# spelunky2json = response.read().decode('utf-8')

spelunky2json = open("Spelunky2.json", "r").read()
j = json.loads(re.sub("//.*", "", spelunky2json, flags=re.MULTILINE))


#default_entity_types = j["default_entity_types"]
entity_class_hierarchy = j["entity_class_hierarchy"]
pointer_types = j["pointer_types"]
inline_struct_types = j["inline_struct_types"]
all_types = j["fields"]

entity_class_hierarchy["Entity"]="Entity" #add missing 'Entity' type for convenience

# Remove the standard types
inline_struct_types.remove("Map")
inline_struct_types.remove("UnorderedMap")
#inline_struct_types.remove("StdVector")
inline_struct_types.remove("StdList")
pointer_types.remove("StdListIteratorPointer")
pointer_types.remove("UnorderedMapBucketPointer")

# ent_base_types = [  "FLOOR_",
                    # "FLOORSTYLED_",
                    # "DECORATION_",
                    # "EMBED_"
                    # "CHAR_"
                    # "MONS_",
                    # "ITEM_",
                    # "ACTIVEFLOOR_",
                    # "FX_",
                    # "BG_",
                    # "MIDBG"
                    # "LOGICAL_",
                    # "MOUNT_",
                    # "LIQUID_"]

cpp_types = {
    "Bool": "bool",
    "Byte": "int8_t",
    "UnsignedByte": "uint8_t",
    "Word": "int16_t",
    "UnsignedWord": "uint16_t",
    "Dword": "int32_t",
    "UnsignedDword": "uint32_t",
    "Qword": "int64_t",
    "UnsignedQword": "uint64_t",
    "Float": "float",
    "Flags8": "uint8_t",
    "Flags16": "uint16_t",
    "Flags32": "uint32_t",
    "State8": "int8_t",
    "State16": "int16_t",
    "State32": "int32_t",
    "CodePointer": "size_t",
    "DataPointer": "size_t",
    "EntityPointer": "Entity*",
    "EntityUID": "int32_t",
    "EntityDBID": "ENT_TYPE",
    "TextureDBID": "TEXTURE",
    "EntityUIDPointer": "size_t",
    "EntityDBPointer": "EntityDB*",
    "ParticleDBPointer": "ParticleDB*",
    "ParticleDBID": "uint32_t",
    "TextureDBPointer": "Texture*",
    "TextureDBID": "int64_t",
    "Vector": "Vector",
    "ConstCharPointer": "const char*",
    "ConstCharPointerPointer": "const char**",
    "UTF16Char": "char16_t",
    "StringsTableID": "STRINGID",
    "CharacterDBID": "uint8_t",
    "Map": "std::map<?,?>",
    "UnorderedMap": "std::unordered_map<?,?>",
    "StdVector": "std::vector<?>",
    "StdList": "std::list<?>",
    "StdListIteratorPointer": "std::list<?>::const_iterator",
    
    
    #"UnorderedMapBucketPointer": "",
}

main_structs = [
"GameManager",
"State",
"SaveGame",
"LevelGen",
"EntityDB",
"ParticleDB",
"TextureDB",
"CharacterDB",
"Online"]

def format_type(type):
    return_type = ""
    if type["type"] == "Skip":
        return cpp_types["Byte"] + " skip[" + str(type["offset"]) + "];"
    elif type["type"] == "UTF16StringFixedSize":
        return "char16_t " + type["field"] + "[" + str(int(type["offset"]/2)) + "];"
    elif type["type"] == "UTF8StringFixedSize":
        return "char " + type["field"] + "[" + str(type["offset"]) + "];" #unsure if correct
    elif type["type"] in inline_struct_types:
        return_type = type["type"]
    elif type["type"] not in cpp_types:
        return_type = type["type"] + "*"
    else:
        return_type = cpp_types[type["type"]]
    return return_type + " " + type["field"] + ";"
    
def write_vars(ent_type, file, remove_item):
    for var in all_types[ent_type]:
        if "vftablefunctions" in var: #exception for Movable
            continue
        file.write("    ")
        if var["type"] == "VirtualFunctionTable" or var["field"] == "__vftable":
            file.write("//")
        file.write(format_type(var))
        if "comment" in var:
            file.write(" //" + var["comment"])
        file.write("\n")
    file.write("};\n\n")
    if remove_item:
        all_types.pop(ent_type)


user_input = input("Generate game structs? (Y/N)")
if user_input == 'Y' or user_input == 'y':
    with open("inline_structs.txt", 'w') as inline_file:
        inline_file.write("\n")
        for struct in inline_struct_types:
            inline_file.write("struct " + struct + "\n{\n")
            write_vars(struct, inline_file, 1)
            
    with open("pointers.txt", 'w') as pointers_file:
        pointers_file.write("\n")
        for struct in pointer_types:
            pointers_file.write("struct " + struct + "\n{\n")
            write_vars(struct, pointers_file, 1)
            
    with open("main_structs.txt", 'w') as main_structs_file:
        main_structs_file.write("\n")
        for struct in main_structs:
            main_structs_file.write("struct " + struct + "\n{\n")
            write_vars(struct, main_structs_file, 1)
    print("DONE")
    print()

user_input2 = input("Generate entities? (Y/N)")
if user_input2 == 'Y' or user_input2 == 'y':
    with open("entities.txt", 'w') as entities_file:
        for ent_class in entity_class_hierarchy:
            entities_file.write("class " + ent_class + " : public " + entity_class_hierarchy[ent_class] + "\n{\n")
            entities_file.write("  public:\n")
            write_vars(ent_class, entities_file, 1)
                
    print("DONE")
    print()

if (user_input == 'Y' or user_input == 'y') and (user_input2 == 'Y' or user_input2 == 'y'):
    print("dumping the 'unused' types into dump.txt")
    with open("dump.txt", 'w') as dump_file:
        for struct in all_types:
            dump_file.write("struct " + struct + "\n{\n")
            write_vars(struct, dump_file, 0)

