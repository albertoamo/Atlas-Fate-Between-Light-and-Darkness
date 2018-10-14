function onSceneStart_scene_coliseo()
	toButton(toEntity(getEntityByName("COL_mesh_terminal02")):getCompByName("button")):setCanBePressed(true);
	toButton(toEntity(getEntityByName("COL_mesh_terminal01")):getCompByName("button")):setCanBePressed(false);
end

function onScenePartialStart_scene_coliseo()
	onSceneStart_scene_coliseo();
	movePlayerToRefPos("col_intro_suelo001", i_ref_pos);
end

function onSceneStart_scene_coliseo_2()
	toButton(toEntity(getEntityByName("COL_mesh_terminal02")):getCompByName("button")):setCanBePressed(false);
	toButton(toEntity(getEntityByName("COL_mesh_terminal01")):getCompByName("button")):setCanBePressed(true);
end

function onScenePartialStart_scene_coliseo_2()
	onSceneStart_scene_coliseo_2();
	movePlayerToRefPos("col_zone_a_suelo002", i_ref_pos);
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"col_zone_a_framedoor001\")):getCompByName(\"door\")):open();", 0.5);
end

function onScenePartialEnd_scene_coliseo()
	i_ref_pos = getPlayerLocalCoordinatesInReferenceTo("col_zone_a_suelo002");
end

function onScenePartialEnd_scene_coliseo_2()
	i_ref_pos = getPlayerLocalCoordinatesInReferenceTo("col_bc_suelo003");
end

function onTriggerEnter_COL_trigger_opendoor_intro_player()
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"col_intro_framedoor\")):getCompByName(\"door\")):open()", 0.5);
	getEntityByName("COL_trigger_opendoor_intro"):destroy();
end

function onTriggerEnter_COL_trigger_opendoor_zonea_player()
	execScriptDelayed("toDoor(toEntity(getEntityByName(\"col_zone_a_framedoor001\")):getCompByName(\"door\")):open()", 0.5);
	getEntityByName("COL_trigger_opendoor_zonea"):destroy();
end

function onTriggerEnter_COL_trigger_corridor_intro_player()
	closeIntroDoor();
	if(cinematicsEnabled and not cinematicCorridorToZoneAExecuted) then
		cinematicCorridorToZoneA();
	end
end

function onTriggerEnter_COL_trigger_corridor_intro01_player()
	closeIntroDoor();
	if(cinematicsEnabled and not cinematicCorridorToZoneAExecuted) then
		cinematicCorridorToZoneA();
	end
end

function onTriggerEnter_COL_trigger_corridor_intro02_player()
	closeIntroDoor();
	if(cinematicsEnabled and not cinematicCorridorToZoneAExecuted) then
		cinematicCorridorToZoneA();
	end
end

function closeIntroDoor()
	intro_door = toDoor(toEntity(getEntityByName("col_intro_framedoor")):getCompByName("door"));
	intro_door:setClosedScript("setCorridorInvisible()");
	intro_door:close();
	getEntityByName("COL_trigger_corridor_intro"):destroy();
	getEntityByName("COL_trigger_corridor_intro01"):destroy();
	getEntityByName("COL_trigger_corridor_intro02"):destroy();
end

function cinematicCorridorToZoneA()

	setInBlackScreen(0.25);
	execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	execScriptDelayed("move(\"The Player\", VEC3(0, 0.989,22),VEC3(0, 0.989,21));",0.27);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Coliseo_Rot_2\", 10.0, \"cinematic\", \"\")", 0.27);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Coliseo_Rot_1\", 0.0, \"cinematic\", \"\")", 0.27);


	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Coliseo_Rot_2\", 0)", 5);
	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Coliseo_Rot_1\", 0)", 5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Coliseo_ZoneA_Door\", 0.0, \"cinematic\", \"\")", 5);

	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Coliseo_ZoneA_Door\", 10)", 8);

	execScriptDelayed("resetMainCameras()",6);
	setCinematicPlayerState(true, "inhibitor_capsules", "");
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 18);
	cinematicCorridorToZoneAExecuted = true;
end

function onTriggerEnter_COL_trigger_corridor_zonea_player()
	closeZoneADoor();
	if(cinematicsEnabled and not cinematicCorridorToBasilicExecuted) then
		cinematicCorridorToBasilic();
	end
end

function onTriggerEnter_COL_trigger_corridor_zonea01_player()
	closeZoneADoor();
	if(cinematicsEnabled and not cinematicCorridorToBasilicExecuted) then
		cinematicCorridorToBasilic();
	end
end

function onTriggerEnter_COL_trigger_corridor_zonea02_player()
	closeZoneADoor();
	if(cinematicsEnabled and not cinematicCorridorToBasilicExecuted) then
		cinematicCorridorToBasilic();
	end
end

function closeZoneADoor()
	zonea_door = toDoor(toEntity(getEntityByName("col_zone_a_framedoor001")):getCompByName("door"));
	zonea_door:setClosedScript("setCorridorInvisible()");
	zonea_door:close();
	getEntityByName("COL_trigger_corridor_zonea"):destroy();
	getEntityByName("COL_trigger_corridor_zonea01"):destroy();
	getEntityByName("COL_trigger_corridor_zonea02"):destroy();
end

function cinematicCorridorToBasilic()

	setInBlackScreen(0.25);
	execScriptDelayed("setOutBlackScreen(0.25);",0.3);
	execScriptDelayed("move(\"The Player\", VEC3(-23, 0.989, 0.4),VEC3(-21, 0.989, -4));",1);
	execScriptDelayed("move(\"The Player\", VEC3(-23, 0.989, 0.4),VEC3(-21, 0.989, 0.4));",1.5);
	execScriptDelayed("blendInCamera(\"Camera_Cinematic_Coliseo_Basilic_Door\", 0.0, \"cinematic\", \"\")", 0.27);

	execScriptDelayed("blendOutCamera(\"Camera_Cinematic_Coliseo_Basilic_Door\", 5)", 4);

	execScriptDelayed("resetMainCameras()",1.25);
	setCinematicPlayerState(true, "inhibitor_capsules", "");
	execScriptDelayed("setCinematicPlayerState(false, \"\")", 9);

	cinematicCorridorToBasilicExecuted = true;

end

function transition_coliseum_to_zone_a(button_handle)
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
	makeVisibleByTag("corridor", true);
	toDoor(toEntity(getEntityByName("col_zone_a_framedoor001")):getCompByName("door")):open();
end

function transition_coliseum_to_courtyard(button_handle)
	execScriptDelayed("disableButton(" .. button_handle .. ", false)", 1);
	makeVisibleByTag("corridor", true);
	toDoor(toEntity(getEntityByName("col_bc_framedoor002")):getCompByName("door")):open();
end

function onTriggerEnter_COL_trigger_corridor_zone_a_player()
	getEntityByName("COL_trigger_corridor_zone_a"):destroy();
	tdoor = toDoor(toEntity(getEntityByName("col_zone_a_framedoor001")):getCompByName("door"));
	tdoor:setClosedScript("destroyColPreloadZoneA()");
	tdoor:close();
end

function destroyColPreloadZoneA()
	destroyPartialScene();
	execScriptDelayed("preloadScene(\"scene_zone_a\")", 0.1);
end

function onTriggerEnter_COL_trigger_corridor_bc_player()
	getEntityByName("COL_trigger_corridor_bc"):destroy();
	tdoor = toDoor(toEntity(getEntityByName("col_bc_framedoor002")):getCompByName("door"));
	tdoor:setClosedScript("destroyColPreloadBC()");
	tdoor:close();
end

function destroyColPreloadBC()
	destroyPartialScene();
	execScriptDelayed("preloadScene(\"scene_basilic_courtyard\")", 0.1);
end