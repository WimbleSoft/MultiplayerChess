"""Point the remaining assets (widgets, DataTable, maps) at the C++ classes after the Blueprints were removed.

This is the one-time migration used for the C++ port; it is kept for reference. It expects:
- the [CoreRedirects] block of the port in Config/DefaultEngine.ini,
- Content/Blueprints already deleted (DT_ObjectMap is recreated on the C++ row struct).

  UnrealEditor-Cmd.exe MultiplayerChess.uproject -run=pythonscript -script="Tools/CppPort/migrate_assets.py"
      -unattended -nosplash -nullrhi -SCCProvider=None
Then run verify_assets.py the same way (set DUMP_OUT to the report path).
"""
import os
import unreal

OUT = os.environ.get("MIGRATE_OUT", "C:/temp/migrate.txt")
log = []


def note(s):
    log.append(s)
    unreal.log("MIGRATE " + s)


registry = unreal.AssetRegistryHelpers.get_asset_registry()

# 1) DataTable: rebuild the rows on the native struct (the old rows were tagged with the user struct's member names).
CSV = """---,Class,SlotName,IsAway,ObjectType
H_Rook_L,"/Script/MultiplayerChess.MCRook","A1","False","Rook"
H_Knight_L,"/Script/MultiplayerChess.MCKnight","B1","False","Knight"
H_Bishop_L,"/Script/MultiplayerChess.MCBishop","C1","False","Bishop"
H_Queen,"/Script/MultiplayerChess.MCQueen","D1","False","Queen"
H_King,"/Script/MultiplayerChess.MCKing","E1","False","King"
H_Bishop_R,"/Script/MultiplayerChess.MCBishop","F1","False","Bishop"
H_Knight_R,"/Script/MultiplayerChess.MCKnight","G1","False","Knight"
H_Rook_R,"/Script/MultiplayerChess.MCRook","H1","False","Rook"
H_Pawn_1,"/Script/MultiplayerChess.MCPawn","A2","False","Pawn"
H_Pawn_2,"/Script/MultiplayerChess.MCPawn","B2","False","Pawn"
H_Pawn_3,"/Script/MultiplayerChess.MCPawn","C2","False","Pawn"
H_Pawn_4,"/Script/MultiplayerChess.MCPawn","D2","False","Pawn"
H_Pawn_5,"/Script/MultiplayerChess.MCPawn","E2","False","Pawn"
H_Pawn_6,"/Script/MultiplayerChess.MCPawn","F2","False","Pawn"
H_Pawn_7,"/Script/MultiplayerChess.MCPawn","G2","False","Pawn"
H_Pawn_8,"/Script/MultiplayerChess.MCPawn","H2","False","Pawn"
A_Rook_L,"/Script/MultiplayerChess.MCRook","H8","True","Rook"
A_Knight_L,"/Script/MultiplayerChess.MCKnight","G8","True","Knight"
A_Bishop_L,"/Script/MultiplayerChess.MCBishop","F8","True","Bishop"
A_Queen,"/Script/MultiplayerChess.MCQueen","D8","True","Queen"
A_King,"/Script/MultiplayerChess.MCKing","E8","True","King"
A_Bishop_R,"/Script/MultiplayerChess.MCBishop","C8","True","Bishop"
A_Knight_R,"/Script/MultiplayerChess.MCKnight","B8","True","Knight"
A_Rook_R,"/Script/MultiplayerChess.MCRook","A8","True","Rook"
A_Pawn_1,"/Script/MultiplayerChess.MCPawn","A7","True","Pawn"
A_Pawn_2,"/Script/MultiplayerChess.MCPawn","B7","True","Pawn"
A_Pawn_3,"/Script/MultiplayerChess.MCPawn","C7","True","Pawn"
A_Pawn_4,"/Script/MultiplayerChess.MCPawn","D7","True","Pawn"
A_Pawn_5,"/Script/MultiplayerChess.MCPawn","E7","True","Pawn"
A_Pawn_6,"/Script/MultiplayerChess.MCPawn","F7","True","Pawn"
A_Pawn_7,"/Script/MultiplayerChess.MCPawn","G7","True","Pawn"
A_Pawn_8,"/Script/MultiplayerChess.MCPawn","H7","True","Pawn"
"""
# The table keeps its row struct as a plain path, which the struct redirect does not cover, so it is recreated
# at the same path on the C++ struct. The old DT_ObjectMap.uasset is deleted from disk before this runs.
DT_PATH = "/Game/Blueprints/Game/DT_ObjectMap"
if unreal.EditorAssetLibrary.does_asset_exist(DT_PATH):
    raise RuntimeError("Delete the old DT_ObjectMap.uasset before running this script")
factory = unreal.DataTableFactory()
factory.set_editor_property("struct", unreal.load_object(None, "/Script/MultiplayerChess.S_ObjectMap"))
dt = unreal.AssetToolsHelpers.get_asset_tools().create_asset("DT_ObjectMap", "/Game/Blueprints/Game", unreal.DataTable, factory)
ok = unreal.DataTableFunctionLibrary.fill_data_table_from_csv_string(dt, CSV)
note("DT created struct=%s filled=%s rows=%d" % (dt.get_editor_property("row_struct"), ok, len(unreal.DataTableFunctionLibrary.get_data_table_row_names(dt))))
note("DT saved=%s" % unreal.EditorAssetLibrary.save_loaded_asset(dt, False))

# 2a) "Set members in struct" nodes keep their visible-pin list by member name. The user struct's member names had
# GUID suffixes, so the visible pins are re-applied by hand (only this node uses a ported struct).
SET_FIELDS_PINS = {
    "/Game/UI/Main/UI_OnlineSettings.UI_OnlineSettings:EventGraph.K2Node_SetFieldsInStruct_0": {"PlayerName"},
}
for node_path, visible in SET_FIELDS_PINS.items():
    unreal.EditorAssetLibrary.load_asset(node_path.split(".")[0])
    node = unreal.find_object(None, node_path)
    if node is None:
        note("node not found %s" % node_path)
        continue
    pins = list(node.get_editor_property("show_pin_for_properties"))
    for i in range(len(pins)):
        pin = pins[i]
        pin.set_editor_property("show_pin", str(pin.get_editor_property("property_name")) in visible)
        pins[i] = pin
    node.set_editor_property("show_pin_for_properties", pins)
    # Setting the array does not rebuild the node's pins. ReparentBlueprint refreshes every node, so the widget is
    # moved to a temporary parent and straight back to UserWidget.
    widget_bp = unreal.EditorAssetLibrary.load_asset(node_path.split(".")[0])
    original_parent = unreal.BlueprintEditorLibrary.get_blueprint_parent_class(widget_bp)
    unreal.BlueprintEditorLibrary.reparent_blueprint(widget_bp, unreal.EditorUtilityWidget.static_class())
    unreal.BlueprintEditorLibrary.reparent_blueprint(widget_bp, original_parent)
    note("reparented %s back to %s" % (widget_bp.get_name(), unreal.BlueprintEditorLibrary.get_blueprint_parent_class(widget_bp).get_name()))
    note("pins on %s: %s" % (node_path.split(":")[-1], [(str(p.get_editor_property("property_name")), p.get_editor_property("show_pin")) for p in node.get_editor_property("show_pin_for_properties")]))

# 2) Widgets and the anim blueprint: recompile against the C++ classes and save.
for data in registry.get_assets_by_path("/Game", recursive=True):
    cls = str(data.asset_class_path.asset_name)
    if cls not in ("WidgetBlueprint", "AnimBlueprint", "Blueprint"):
        continue
    path = str(data.package_name)
    bp = unreal.EditorAssetLibrary.load_asset(path)
    if bp is None:
        note("load failed %s" % path)
        continue
    unreal.BlueprintEditorLibrary.refresh_all_nodes(bp) if hasattr(unreal.BlueprintEditorLibrary, "refresh_all_nodes") else None
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    status = None
    try:
        status = bp.get_editor_property("status")
    except Exception:
        pass
    note("compiled %s status=%s" % (path, status))
    unreal.EditorAssetLibrary.save_loaded_asset(bp, False)

# 3) Maps: load, rebuild the board through its construction script, save.
for m in ["/Game/Maps/Game", "/Game/Maps/Test", "/Game/Maps/Lobby", "/Game/Maps/MainMenu", "/Game/Maps/Transition"]:
    world = unreal.EditorLoadingAndSavingUtils.load_map(m)
    if not world:
        note("map load failed %s" % m)
        continue
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    # Pick the hand-placed actors first: the board's squares and pieces are child actors and are rebuilt by the board.
    all_actors = unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor)
    candidates = [a for a in all_actors if a.get_class().get_name().startswith("MC") and a.get_parent_actor() is None]
    for a in candidates:
        c = a.get_class().get_name()
        # The old board's child actors don't go away with it, so remove them explicitly.
        old_children = [x for x in all_actors if x.get_parent_actor() == a]
        for child in old_children:
            actor_subsystem.destroy_actor(child)
        if old_children:
            note("%s: removed %d child actors of the old %s" % (m, len(old_children), a.get_actor_label()))
        # Actors placed from the old Blueprints still carry the Blueprint's serialized components.
        # Replace them with a fresh instance of the C++ class at the same place.
        label = a.get_actor_label()
        transform = a.get_actor_transform()
        is_away = a.get_editor_property("is_away") if c != "MCChessTable" else None
        new_actor = actor_subsystem.spawn_actor_from_class(a.get_class(), transform.translation, transform.rotation.rotator())
        new_actor.set_actor_scale3d(transform.scale3d)
        if is_away is not None:
            new_actor.set_editor_property("is_away", is_away)
        actor_subsystem.destroy_actor(a)
        new_actor.set_actor_label(label)
        note("%s: replaced %s (%s)" % (m, label, c))
        if c == "MCChessTable":
            note("  AllSlots=%d AllObjects=%d" % (len(new_actor.get_editor_property("all_slots")), len(new_actor.get_editor_property("all_objects"))))
    ws = world.get_world_settings()
    note("  game mode override: %s" % ws.get_editor_property("default_game_mode"))
    note("  saved=%s" % unreal.EditorLoadingAndSavingUtils.save_map(world, m))

with open(OUT, "w", encoding="utf-8") as f:
    f.write("\n".join(log))
