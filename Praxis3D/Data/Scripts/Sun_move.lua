
function init ()
	-- Create needed variables
	create(Types.InputVariables, 'inputVariables');
	create(Types.SpatialDataManager, 'spatialData');
		
	enabled = false
	--moveSpeed = 2.0
	
	if sunMoveSpeed then
		moveSpeed = sunMoveSpeed
	else
		moveSpeed = 5.0
	end
	
	create(Types.KeyCommand, 'forwardKey')
	create(Types.KeyCommand, 'backwardKey')
	
	forwardKey:bind(inputVariables.debug_1_key)
	backwardKey:bind(inputVariables.debug_2_key)
	
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
end
	
function update (p_deltaTime)
	angleChanged = false
	
	spatialData:calculateLocalRotationEuler()
	eulerAngle = spatialData:getLocalSpaceData().m_spatialData.m_rotationEuler
	
	if forwardKey:isActivated() then
		eulerAngle.x = eulerAngle.x + (moveSpeed * p_deltaTime)
		--eulerAngle.y = eulerAngle.y + (moveSpeed * p_deltaTime)
		angleChanged = true
	end	
	
	if backwardKey:isActivated() then
		eulerAngle.x = eulerAngle.x - (moveSpeed * p_deltaTime)
		--eulerAngle.y = eulerAngle.y - (moveSpeed * p_deltaTime)
		angleChanged = true
	end	
	
	if angleChanged then
		spatialData:setLocalRotationEuler(eulerAngle)
		--spatialData:update()
		--spatialData:calculateLocalRotationQuaternion()
	end
	
	--print(eulerAngle.x)
	
	--postChanges(Changes.GUI.Sequence)
	
	--print('test')
end