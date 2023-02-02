
function init ()
	-- Create needed variables
	create(Types.EngineVariables, 'engineVariables');
	create(Types.GraphicsVariables, 'graphicsVariables');
	create(Types.InputVariables, 'inputVariables');
	create(Types.WindowVariables, 'windowVariables');
	
	create(Types.Conditional, 'loadButtonPressed')
	create(Types.Conditional, 'playButtonPressed')
	create(Types.Conditional, 'optionsButtonPressed')
	create(Types.Conditional, 'aboutButtonPressed')
	create(Types.Conditional, 'exitButtonPressed')
	create(Types.Conditional, 'textureButton')
	
	praxisLogoTexture = loadTexture2D('PraxisLogoFinal.png')
	praxisLogoTexture:loadToMemory()
	praxisLogoTexture:loadToVideoMemory()
	
	praxisLogoScale = 1
	
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
	
	-- Calculate the position of the middle of the screen
	halfScreenSizeX = graphicsVariables.current_resolution_x / 2.0
	halfScreenSizeY = graphicsVariables.current_resolution_y / 2.0
	
	-- Calculate the starting position for the buttons (relative to window size, not absolute, so the buttons are always in the right place)
	buttonPositionX = halfScreenSizeX * buttonOffsetMultX
	buttonPositionY = halfScreenSizeY * buttonOffsetMultY
	
	-- Calculate logo size based in texture and window size (try to fit it on screen vertically)
	logoAdjustedSizeX = praxisLogoTexture:getTextureWidth() / (praxisLogoTexture:getTextureHeight() / graphicsVariables.current_resolution_y) * praxisLogoScale
	logoAdjustedSizeY = praxisLogoTexture:getTextureHeight() / (praxisLogoTexture:getTextureHeight() / graphicsVariables.current_resolution_y) * praxisLogoScale

	-- Draw the background color
	GUI.PushStyleColor(ImGuiCol.WindowBg, 0.102, 0.102, 0.102, 255.0)
	GUI.SetNextWindowPos(0, 0)
	GUI.SetNextWindowSize(graphicsVariables.current_resolution_x, graphicsVariables.current_resolution_y)
	GUI.Begin('BACKGROUND', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoMouseInputs, ImGuiWindowFlags.NoFocusOnAppearing))
	GUI.End()
	GUI.PopStyleColor()
	
	-- Draw the engine logo
	GUI.SetNextWindowPos((graphicsVariables.current_resolution_x - logoAdjustedSizeX) / 2, (graphicsVariables.current_resolution_y - logoAdjustedSizeY) / 2)
	GUI.SetNextWindowSize(logoAdjustedSizeX, logoAdjustedSizeY)
	GUI.Begin('PRAXIS LOGO', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground, ImGuiWindowFlags.NoMouseInputs))
	GUI.Image(praxisLogoTexture, logoAdjustedSizeX, logoAdjustedSizeY)
	GUI.End()
	
	-- Draw the EXIT button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY)
	GUI.Begin('EXIT BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('EXIT', buttonSizeX, buttonSizeY, exitButtonPressed)
	GUI.End()
	
	-- Draw the ABOUT button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - (buttonSpacing + buttonSizeY))
	GUI.Begin('ABOUT BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('ABOUT', buttonSizeX, buttonSizeY, aboutButtonPressed)
	GUI.End()
	
	-- Draw the OPTIONS button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 2))
	GUI.Begin('OPTIONS BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('OPTIONS', buttonSizeX, buttonSizeY, optionsButtonPressed)
	GUI.End()
	
	-- Draw the PLAY button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 3))
	GUI.Begin('PLAY BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('PLAY', buttonSizeX, buttonSizeY, playButtonPressed)
	GUI.End()
	
	-- Draw the LOAD MAP button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 4))
	GUI.Begin('LOAD BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	GUI.Button('LOAD MAP', buttonSizeX, buttonSizeY, loadButtonPressed)
	GUI.End()
	
	-- Check if the EXIT button is pressed; close the engine if it is
	if exitButtonPressed:isChecked() then
		print('Exit called from MainMenu.lua')
		setEngineRunning(false)
	end
	
	-- Check if the OPTIONS button is pressed
	if aboutButtonPressed:isChecked() then
		print('ABOUT')
	end
	
	-- Check if the OPTIONS button is pressed
	if optionsButtonPressed:isChecked() then
		print('OPTIONS')
	end
	
	-- Check if the PLAY button is pressed; change the current engine state to PLAY, if it is
	if playButtonPressed:isChecked() then
		setEngineState(EngineStateType.Play)
		setMouseCapture(true)
	end
	
	-- Check if the LOAD button is pressed
	if loadButtonPressed:isChecked() then
		print('LOAD')
	end
	
end