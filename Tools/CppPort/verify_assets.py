"""Reload everything after the migration and report compile state, map contents and leftover references."""
import os
import unreal

out = []
reg = unreal.AssetRegistryHelpers.get_asset_registry()
bad_refs = set()
for data in reg.get_assets_by_path("/Game", recursive=True):
    path = str(data.package_name)
    cls = str(data.asset_class_path.asset_name)
    deps = reg.get_dependencies(path, unreal.AssetRegistryDependencyOptions())
    for d in deps or []:
        d = str(d)
        if d.startswith("/Game/Blueprints/") and not d.endswith("DT_ObjectMap"):
            bad_refs.add("%s -> %s" % (path, d))
    if cls in ("WidgetBlueprint", "AnimBlueprint", "Blueprint"):
        bp = unreal.EditorAssetLibrary.load_asset(path)
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        out.append("%s %s" % (bp.get_editor_property("status"), path))
out.append("references to removed Blueprint packages: %d" % len(bad_refs))
out += sorted(bad_refs)
for m in ["/Game/Maps/Game", "/Game/Maps/Test"]:
    world = unreal.EditorLoadingAndSavingUtils.load_map(m)
    counts = {}
    for a in unreal.GameplayStatics.get_all_actors_of_class(world, unreal.Actor):
        c = a.get_class().get_name()
        if c.startswith("MC"):
            counts[c] = counts.get(c, 0) + 1
    out.append("%s: %s" % (m, counts))
dt = unreal.EditorAssetLibrary.load_asset("/Game/Blueprints/Game/DT_ObjectMap")
out.append("DT rows=%d struct=%s" % (len(unreal.DataTableFunctionLibrary.get_data_table_row_names(dt)), dt.get_editor_property("row_struct").get_name()))
open(os.environ["DUMP_OUT"], "w", encoding="utf-8").write("\n".join(out))
