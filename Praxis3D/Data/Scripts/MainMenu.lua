
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
	
	-- Engine logo texture
	praxisLogoTexture = loadTexture2D('PraxisLogoFinal.png')
	praxisLogoTexture:loadToMemory()
	praxisLogoTexture:loadToVideoMemory()
	
	-- Fmod logo texture
	fmodLogoTexture = loadTexture2D('FMOD Logo White - Black Background.png')
	fmodLogoTexture:loadToMemory()
	fmodLogoTexture:loadToVideoMemory()
	
	praxisLogoScale = 1
	fmodLogoScale = 0.05
	
	buttonSizeX = 100.0
	buttonSizeY = 25.0
	
	buttonHalfSizeX = buttonSizeX / 2
	buttonHalfSizeY = buttonSizeY / 2
	
	buttonOffsetMultX = 1
	buttonOffsetMultY = 1.75
	
	buttonSpacing = 5
	
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
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
	
	-- Calculate engine logo size based based on texture and window size (try to fit it on screen vertically)
	engineLogoAdjustedSizeX = praxisLogoTexture:getTextureWidth() / (praxisLogoTexture:getTextureWidth() / graphicsVariables.current_resolution_x) * praxisLogoScale
	engineLogoAdjustedSizeY = praxisLogoTexture:getTextureHeight() / (praxisLogoTexture:getTextureWidth() / graphicsVariables.current_resolution_x) * praxisLogoScale

	-- Calculate fmod logo size based on texture and window size (so it scaled with the screen size)
	fmodLogoAdjustedSizeMultAverage = ((fmodLogoTexture:getTextureHeight() / graphicsVariables.current_resolution_y) + (fmodLogoTexture:getTextureWidth() / graphicsVariables.current_resolution_x)) / 2
	fmodLogoAdjustedSizeX = fmodLogoTexture:getTextureWidth() / fmodLogoAdjustedSizeMultAverage * fmodLogoScale
	fmodLogoAdjustedSizeY = fmodLogoTexture:getTextureHeight() / fmodLogoAdjustedSizeMultAverage * fmodLogoScale
	
	-- Remove window padding and border size, so the content inside the window fills the whole window
	GUI.PushStyleVar(ImGuiStyleVar.WindowPadding, 0.0, 0.0)
	GUI.PushStyleVar(ImGuiStyleVar.WindowBorderSize, 0.0)
	
	-- Draw the background color
	GUI.PushStyleColor(ImGuiCol.WindowBg, 0.102, 0.102, 0.102, 255.0)
	GUI.SetNextWindowPos(0, 0)
	GUI.SetNextWindowSize(graphicsVariables.current_resolution_x, graphicsVariables.current_resolution_y)
	GUI.Begin('BACKGROUND', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoMouseInputs, ImGuiWindowFlags.NoFocusOnAppearing))
	GUI.End()
	GUI.PopStyleColor()
	
	-- Draw the engine logo
	GUI.SetNextWindowPos((graphicsVariables.current_resolution_x - engineLogoAdjustedSizeX) / 2, (graphicsVariables.current_resolution_y - engineLogoAdjustedSizeY) / 2)
	GUI.SetNextWindowSize(engineLogoAdjustedSizeX, engineLogoAdjustedSizeY)
	GUI.Begin('PRAXIS LOGO', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground, ImGuiWindowFlags.NoMouseInputs))
	GUI.Image(praxisLogoTexture, engineLogoAdjustedSizeX, engineLogoAdjustedSizeY)
	GUI.End()
	
	-- Draw the fmod logo
	GUI.SetNextWindowPos((halfScreenSizeX - (fmodLogoAdjustedSizeX / 2)) * fmodLogoOffsetMultX, (halfScreenSizeY - (fmodLogoAdjustedSizeY / 2)) * fmodLogoOffsetMultY)
	GUI.SetNextWindowSize(fmodLogoAdjustedSizeX, fmodLogoAdjustedSizeY)
	GUI.Begin('FMOD LOGO', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground, ImGuiWindowFlags.NoMouseInputs))
	GUI.Image(fmodLogoTexture, fmodLogoAdjustedSizeX, fmodLogoAdjustedSizeY)
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
	
	-- Pop the removal of frame padding and border size
	GUI.PopStyleVar(2)
	
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