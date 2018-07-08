function onSceneStart_map_intro()
	--playerController = getPlayerController();	--no need. We will do it always in each loading scene
	--blendInCamera("Camera Follow Drone", 0);
end

function transition_map_intro_to_zone_a()
	execScriptDelayed("blendInCamera(\"scene_transition\", 1.0)", 2);
	execScriptDelayed("pausePlayerToggle()", 2);
	execScriptDelayed("cinematicModeToggle()", 2);
	execScriptDelayed("loadScene(\"scene_zone_a\")", 5);
end