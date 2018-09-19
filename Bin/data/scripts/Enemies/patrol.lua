function animation_LaunchInhibitor(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:launchInhibitor();
end

function animation_Attack(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:attackPlayer();
end

function animation_Patrol_Walk_Left(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:playStepParticle(true);
end

function animation_Patrol_Walk_Right(sender)
	h = CHandle();
	h:fromUnsigned(sender);
	e_patrol = toEntity(h);
	ai_patrol = toAIPatrol(e_patrol:getCompByName("ai_patrol"));
	ai_patrol:playStepParticle(false);
end