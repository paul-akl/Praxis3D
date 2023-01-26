
function init ()
	-- Create needed variables
	create(Types.GameplayVariables, 'gameplayVariables');
	create(Types.InputVariables, 'inputVariables');
	create(Types.SpatialDataManager, 'spatialData');
	
	-- Create key commands, used to track pressed keys
	create(Types.KeyCommand, 'forwardKey')
	create(Types.KeyCommand, 'upKey')
	create(Types.KeyCommand, 'downKey')
	
	-- Bind keys to their corresponding buttons on the keyboard
	forwardKey:bind(inputVariables.forward_key)
	upKey:bind(inputVariables.up_editor_key)
	downKey:bind(inputVariables.down_editor_key)
	
	print('Kinematic_test.lua script initialized.')
end

function update (p_deltaTime)
	-- Get current spatial data
	localTransformMat4 = spatialData:getLocalTransform()
	
	-- Extract position from spatial data
	positionVec3 = localTransformMat4:getPosVec3()
	
	positionModified = false
	
	spawnEntity = false
		
	if upKey:isActivated() then
		positionVec3.y = positionVec3.y + (10.0 * p_deltaTime)
		positionModified = true
	end	
	if downKey:isActivated() then
		positionVec3.y = positionVec3.y - (10.0 * p_deltaTime)
		positionModified = true
	end
		
	if(positionModified) then
		spatialData:setLocalPosition(positionVec3)
	end
	
end