
function init ()
	-- Create needed variables
	create(Types.EngineVariables, 'engineVariables');
	create(Types.GraphicsVariables, 'graphicsVariables');
	create(Types.InputVariables, 'inputVariables');
	create(Types.WindowVariables, 'windowVariables');
	
	create(Types.Conditional, 'loadButton')
	create(Types.Conditional, 'playButton')
	create(Types.Conditional, 'optionsButton')
	create(Types.Conditional, 'exitButton')
	create(Types.Conditional, 'checkbox1')
	
	buttonSizeX = 100.0
	buttonSizeY = 25.0
	
	buttonHalfSizeX = buttonSizeX / 2
	buttonHalfSizeY = buttonSizeY / 2
	
	buttonOffsetMultX = 1
	buttonOffsetMultY = 1.75
	
	buttonSpacing = 5
	
	print('MainMenu.lua script initialized.')
end
	
function update (p_deltaTime)
	-- Make sure the mouse is released, so the buttons can be pressed
	setMouseCapture(false)
	
	-- Calculate the starting position for the buttons (relative to window size, not absolute, so the buttons are always in the right place)
	halfScreenSizeX = (graphicsVariables.current_resolution_x / 2.0) * buttonOffsetMultX
	halfScreenSizeY = (graphicsVariables.current_resolution_y / 2.0) * buttonOffsetMultY
	
	-- Draw the EXIT button
	GUI.SetNextWindowPos(halfScreenSizeX - buttonHalfSizeX, halfScreenSizeY - buttonHalfSizeY)
	GUI.Begin('EXIT BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('EXIT', buttonSizeX, buttonSizeY, exitButton)
	GUI.End()
	
	-- Draw the OPTIONS button
	GUI.SetNextWindowPos(halfScreenSizeX - buttonHalfSizeX, halfScreenSizeY - buttonHalfSizeY - (buttonSpacing + buttonSizeY))
	GUI.Begin('OPTIONS BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('OPTIONS', buttonSizeX, buttonSizeY, optionsButton)
	GUI.End()
	
	-- Draw the PLAY button
	GUI.SetNextWindowPos(halfScreenSizeX - buttonHalfSizeX, halfScreenSizeY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 2))
	GUI.Begin('PLAY BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('PLAY', buttonSizeX, buttonSizeY, playButton)
	GUI.End()
	
	-- Draw the LOAD MAP button
	GUI.SetNextWindowPos(halfScreenSizeX - buttonHalfSizeX, halfScreenSizeY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 3))
	GUI.Begin('LOAD BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('LOAD MAP', buttonSizeX, buttonSizeY, loadButton)
	GUI.End()
	
	-- Check if the EXIT button is pressed; close the engine if it is
	if exitButton:isChecked() then
		print('Exit called from MainMenu.lua')
		setEngineRunning(false)
	end
	
	-- Check if the OPTIONS button is pressed
	if optionsButton:isChecked() then
		print('OPTIONS')
	end
	
	-- Check if the PLAY button is pressed; change the current engine state to PLAY, if it is
	if playButton:isChecked() then
		setEngineState(EngineStateType.Play)
		setMouseCapture(true)
	end
	
	-- Check if the LOAD button is pressed
	if loadButton:isChecked() then
		print('LOAD')
	end
	
	--print('test')
end