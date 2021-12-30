
print('before functions call in LUA')

function init ()
	print('init() function call in LUA')
	
	create(Types.SpatialDataManager, 'spatialData');
	
	create(Types.InputVariables, 'inputVariables');
	
	create(Types.KeyCommand, 'forwardKey')
	create(Types.KeyCommand, 'backwardKey')
	create(Types.KeyCommand, 'leftKey')
	create(Types.KeyCommand, 'rightKey')
	create(Types.KeyCommand, 'upKey')
	create(Types.KeyCommand, 'downKey')
	
	forwardKey:bind(inputVariables.forward_key)
	backwardKey:bind(inputVariables.backward_key)
	leftKey:bind(inputVariables.left_strafe_key)
	rightKey:bind(inputVariables.right_strafe_key)
	upKey:bind(inputVariables.up_key)
	downKey:bind(inputVariables.down_key)
	
	x = 0
	y = 0
	speed = 100
	
	vec1 = Vec3f.new()
	vec1.x = 5
	vec1.y = 8
	
	vec2 = Vec3f.new()
	vec2.x = 10
	vec2.y = 20
end

function update (p_deltaTime)
	--print('update() function call in LUA')
	
	
	--test1 = spatialData:getWorldSpaceData
	
	--targetVec = test1.m_spatialData.m_rotationEuler
	
	--targetVec = Vec3f.new()
	--targetVec.x = 99
	--targetVec.y = 99
	
	--targetVec = spatialData:getWorldRotation()
	
	positionVec = spatialData:getWorldSpaceData().m_spatialData.m_position
	rotationVec = spatialData:getWorldSpaceData().m_spatialData.m_rotationEuler
	targetVec = Vec3f.new()
	targetVec:target(toRadianF(rotationVec.x), toRadianF(rotationVec.y))
	
	if forwardKey:isActivated() then
		positionVec:subVec3f(targetVec:mulF(speed * p_deltaTime))
		--x = x + (speed * p_deltaTime)
	end	
	
	if backwardKey:isActivated() then
		positionVec:addVec3f(targetVec:mulF(speed * p_deltaTime))
		--x = x - (speed * p_deltaTime)
	end
	
	if leftKey:isActivated() then
		y = y + (speed * p_deltaTime)
	end	
	
	if rightKey:isActivated() then
		y = y - (speed * p_deltaTime)
	end
	
	--x = vec1.x
	--y = vec1.y
	
	vec1.x = x
	vec1.y = y
	
	spatialData:setWorldPosition(positionVec)
	
	
	--vec1:mulVec3f(vec2)
	--print(Types.KeyCommand)
	--print('test')
	--print(vec1.x .. ' : ' .. vec1.y)
end