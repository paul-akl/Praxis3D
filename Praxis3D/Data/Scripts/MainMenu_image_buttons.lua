--[[
	The default main-menu screen of the engine.
	
	Contains full-screen engine logo, and the required FMOD logo.
	Uses textures for buttons.
	Spawns file-browser-dialog for loading scene files.
	Scales with any screen size.
]]--

function init ()
	-- Create needed variables
	create(Types.EngineVariables, 'engineVariables');
	create(Types.GraphicsVariables, 'graphicsVariables');
	create(Types.InputVariables, 'inputVariables');
	create(Types.PathsVariables, 'pathsVariables');
	create(Types.WindowVariables, 'windowVariables');
	
	create(Types.Conditional, 'loadButtonPressed')
	create(Types.Conditional, 'loadButtonHover')
	
	create(Types.Conditional, 'playButtonPressed')
	create(Types.Conditional, 'playButtonHover')
	
	create(Types.Conditional, 'editorButtonPressed')
	create(Types.Conditional, 'editorButtonHover')
	
	create(Types.Conditional, 'optionsButtonPressed')
	create(Types.Conditional, 'optionsButtonHover')
	
	create(Types.Conditional, 'aboutButtonPressed')
	create(Types.Conditional, 'aboutButtonHover')
	
	create(Types.Conditional, 'exitButtonPressed')
	create(Types.Conditional, 'exitButtonHover')
	
	-- Load button textures
	buttonLoadTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_load_0.png')
	buttonLoadTexture:setEnableMipmapping(false)
	buttonLoadTexture:setEnableDownsampling(false)
	buttonLoadTexture:loadToMemory()
	buttonLoadTexture:loadToVideoMemory()
	
	buttonLoadPressedTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_load_1.png')
	buttonLoadPressedTexture:setEnableMipmapping(false)
	buttonLoadPressedTexture:setEnableDownsampling(false)
	buttonLoadPressedTexture:loadToMemory()
	buttonLoadPressedTexture:loadToVideoMemory()
	
	-- Play button textures
	buttonPlayTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_play_0.png')
	buttonPlayTexture:setEnableMipmapping(false)
	buttonPlayTexture:setEnableDownsampling(false)
	buttonPlayTexture:loadToMemory()
	buttonPlayTexture:loadToVideoMemory()
	
	buttonPlayPressedTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_play_1.png')
	buttonPlayPressedTexture:setEnableMipmapping(false)
	buttonPlayPressedTexture:setEnableDownsampling(false)
	buttonPlayPressedTexture:loadToMemory()
	buttonPlayPressedTexture:loadToVideoMemory()
	
	-- Editor button textures
	buttonEditorTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_editor_0.png')
	buttonEditorTexture:setEnableMipmapping(false)
	buttonEditorTexture:setEnableDownsampling(false)
	buttonEditorTexture:loadToMemory()
	buttonEditorTexture:loadToVideoMemory()
	
	buttonEditorPressedTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_editor_1.png')
	buttonEditorPressedTexture:setEnableMipmapping(false)
	buttonEditorPressedTexture:setEnableDownsampling(false)
	buttonEditorPressedTexture:loadToMemory()
	buttonEditorPressedTexture:loadToVideoMemory()
	
	-- Options button textures
	buttonOptionsTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_options_0.png')
	buttonOptionsTexture:setEnableMipmapping(false)
	buttonOptionsTexture:setEnableDownsampling(false)
	buttonOptionsTexture:loadToMemory()
	buttonOptionsTexture:loadToVideoMemory()
	
	buttonOptionsPressedTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_options_1.png')
	buttonOptionsPressedTexture:setEnableMipmapping(false)
	buttonOptionsPressedTexture:setEnableDownsampling(false)
	buttonOptionsPressedTexture:loadToMemory()
	buttonOptionsPressedTexture:loadToVideoMemory()
	
	-- About button textures
	buttonAboutTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_about_0.png')
	buttonAboutTexture:setEnableMipmapping(false)
	buttonAboutTexture:setEnableDownsampling(false)
	buttonAboutTexture:loadToMemory()
	buttonAboutTexture:loadToVideoMemory()
	
	buttonAboutPressedTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_about_1.png')
	buttonAboutPressedTexture:setEnableMipmapping(false)
	buttonAboutPressedTexture:setEnableDownsampling(false)
	buttonAboutPressedTexture:loadToMemory()
	buttonAboutPressedTexture:loadToVideoMemory()
	
	-- Exit button textures
	buttonExitTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_exit_0.png')
	buttonExitTexture:setEnableMipmapping(false)
	buttonExitTexture:setEnableDownsampling(false)
	buttonExitTexture:loadToMemory()
	buttonExitTexture:loadToVideoMemory()
	
	buttonExitPressedTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'button_exit_1.png')
	buttonExitPressedTexture:setEnableMipmapping(false)
	buttonExitPressedTexture:setEnableDownsampling(false)
	buttonExitPressedTexture:loadToMemory()
	buttonExitPressedTexture:loadToVideoMemory()
	
	-- Engine logo texture
	praxisLogoTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'logo1.png')
	praxisLogoTexture:setEnableMipmapping(false)
	praxisLogoTexture:setEnableDownsampling(false)
	praxisLogoTexture:loadToMemory()
	praxisLogoTexture:loadToVideoMemory()
	
	-- Fmod logo texture
	fmodLogoTexture = loadTexture2D(pathsVariables.gui_assets_path .. 'FMOD Logo White - Black Background.png')
	fmodLogoTexture:setEnableMipmapping(false)
	fmodLogoTexture:setEnableDownsampling(false)
	fmodLogoTexture:loadToMemory()
	fmodLogoTexture:loadToVideoMemory()
	
	praxisLogoScale = 1
	fmodLogoScale = 0.05
	
	fmodLogoOffsetMultX = 0.05
	fmodLogoOffsetMultY = 1.95
	
	--buttonSizeX = 100.0
	--buttonSizeY = 25.0
	buttonSizeX = buttonExitTexture:getTextureWidth()
	buttonSizeY = buttonExitTexture:getTextureHeight()
	
	buttonHalfSizeX = buttonSizeX / 2
	buttonHalfSizeY = buttonSizeY / 2
	
	buttonOffsetMultX = 1.75
	buttonOffsetMultY = 1.75
	
	buttonSpacing = 5
	
	fileBrowserLoadMap = FileBrowserDialog.new()
	fileBrowserLoadMap.m_name = 'fileBrowserLoadMap'
	fileBrowserLoadMap.m_title = 'Load Map File'
	fileBrowserLoadMap.m_filter = '.pmap,*.*'
	fileBrowserLoadMap.m_rootPath = pathsVariables.map_path
	
	fileBrowserEditMap = FileBrowserDialog.new()
	fileBrowserEditMap.m_name = 'fileBrowserEditMap'
	fileBrowserEditMap.m_title = 'Edit Map File'
	fileBrowserEditMap.m_filter = '.pmap,*.*'
	fileBrowserEditMap.m_rootPath = pathsVariables.map_path
	
	fileBrowserLoadMapOpened = false
	fileBrowserEditMapOpened = false
	
	ErrHandlerLoc.logErrorCode(ErrorCode.Initialize_success, getLuaFilename())
end
	
function drawLoadingScreen()
	-- Draw the background color
	GUI.PushStyleColor(ImGuiCol.WindowBg, 0.102, 0.102, 0.102, 255.0)
	GUI.SetNextWindowPos(0, 0)
	GUI.SetNextWindowSize(screenSize.x, screenSize.y)
	GUI.Begin('##LoadingScreen', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoMouseInputs))
	
	-- Set the CONTINUE button position to be centered at the bottom of the screen
	GUI.SetCursorPosX((screenSize.x) * 0.5)
	GUI.SetCursorPosY((screenSize.y) * 0.5)
	GUI.Text('Loading...')
	
	GUI.End()
	GUI.PopStyleColor()
end
	
function update (p_deltaTime)
	-- Make sure the mouse is released, so the buttons can be pressed
	setMouseCapture(false)
	
	-- Get the current view-port size
	screenSize = GUI.GetScreenSize()
	
	-- Calculate the position of the middle of the screen
	halfScreenSizeX = screenSize.x / 2.0
	halfScreenSizeY = screenSize.y / 2.0
	
	-- Calculate the starting position for the buttons (relative to window size, not absolute, so the buttons are always in the right place)
	buttonPositionX = halfScreenSizeX * buttonOffsetMultX
	buttonPositionY = halfScreenSizeY * buttonOffsetMultY
	
	-- Calculate engine logo size based based on texture and window size (try to fit it on screen vertically)
	engineLogoAdjustedSizeX = praxisLogoTexture:getTextureWidth() / (praxisLogoTexture:getTextureWidth() / screenSize.x) * praxisLogoScale
	engineLogoAdjustedSizeY = praxisLogoTexture:getTextureHeight() / (praxisLogoTexture:getTextureWidth() / screenSize.x) * praxisLogoScale
	
	-- Calculate fmod logo size based on texture and window size (so it scaled with the screen size)
	fmodLogoAdjustedSizeMultAverage = ((fmodLogoTexture:getTextureHeight() / screenSize.y) + (fmodLogoTexture:getTextureWidth() / screenSize.x)) / 2
	fmodLogoAdjustedSizeX = fmodLogoTexture:getTextureWidth() / fmodLogoAdjustedSizeMultAverage * fmodLogoScale
	fmodLogoAdjustedSizeY = fmodLogoTexture:getTextureHeight() / fmodLogoAdjustedSizeMultAverage * fmodLogoScale

	-- Remove window padding and border size, so the content inside the window fills the whole window
	GUI.PushStyleVar(ImGuiStyleVar.WindowPadding, 0.0, 0.0)
	GUI.PushStyleVar(ImGuiStyleVar.WindowBorderSize, 0.0)

	-- Draw the background color
	GUI.PushStyleColor(ImGuiCol.WindowBg, 0.102, 0.102, 0.102, 255.0)
	GUI.SetNextWindowPos(0, 0)
	GUI.SetNextWindowSize(screenSize.x, screenSize.y)
	GUI.Begin('BACKGROUND', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoMouseInputs, ImGuiWindowFlags.NoFocusOnAppearing))
	GUI.End()
	GUI.PopStyleColor()
	
	-- Draw the engine logo
	GUI.SetNextWindowPos((screenSize.x - engineLogoAdjustedSizeX) / 2, (screenSize.y - engineLogoAdjustedSizeY) / 2)
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
	
	-- Push transparent background colors for all buttons
	GUI.PushStyleColor(ImGuiCol.Button, 0.0, 0.0, 0.0, 0.0);
	GUI.PushStyleColor(ImGuiCol.ButtonActive, 0.0, 0.0, 0.0, 0.0);
	GUI.PushStyleColor(ImGuiCol.ButtonHovered, 0.0, 0.0, 0.0, 0.0);
		
	-- Draw the EXIT button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY)
	GUI.Begin('EXIT BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	if exitButtonHover:isChecked() then
		GUI.ImageButton(buttonExitPressedTexture, exitButtonPressed)
	else
		GUI.ImageButton(buttonExitTexture, exitButtonPressed)
	end
	GUI.IsItemHovered(exitButtonHover)
	GUI.End()
	
	-- Draw the ABOUT button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - (buttonSpacing + buttonSizeY))
	GUI.Begin('ABOUT BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	if aboutButtonHover:isChecked() then
		GUI.ImageButton(buttonAboutPressedTexture, aboutButtonPressed)
	else
		GUI.ImageButton(buttonAboutTexture, aboutButtonPressed)
	end
	GUI.IsItemHovered(aboutButtonHover)
	GUI.End()
	
	-- Draw the OPTIONS button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 2))
	GUI.Begin('OPTIONS BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	if optionsButtonHover:isChecked() then
		GUI.ImageButton(buttonOptionsPressedTexture, optionsButtonPressed)
	else
		GUI.ImageButton(buttonOptionsTexture, optionsButtonPressed)
	end
	GUI.IsItemHovered(optionsButtonHover)
	GUI.End()
	
	-- Draw the EDIT button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 3))
	GUI.Begin('EDITOR BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	if editorButtonHover:isChecked() then
		GUI.ImageButton(buttonEditorPressedTexture, editorButtonPressed)
	else
		GUI.ImageButton(buttonEditorTexture, editorButtonPressed)
	end
	GUI.IsItemHovered(editorButtonHover)
	GUI.End()
	
	-- Draw the PLAY button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 4))
	GUI.Begin('PLAY BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	if playButtonHover:isChecked() then
		GUI.ImageButton(buttonPlayPressedTexture, playButtonPressed)
	else
		GUI.ImageButton(buttonPlayTexture, playButtonPressed)
	end
	GUI.IsItemHovered(playButtonHover)
	GUI.End()
	
	-- Draw the LOAD MAP button
	GUI.SetNextWindowPos(buttonPositionX - buttonHalfSizeX, buttonPositionY - buttonHalfSizeY - ((buttonSpacing + buttonSizeY) * 5))
	GUI.Begin('LOAD BTN', bitwiseOr(ImGuiWindowFlags.NoDecoration, ImGuiWindowFlags.NoMove, ImGuiWindowFlags.NoSavedSettings, ImGuiWindowFlags.NoBackground))
	if loadButtonHover:isChecked() then
		GUI.ImageButton(buttonLoadPressedTexture, loadButtonPressed)
	else
		GUI.ImageButton(buttonLoadTexture, loadButtonPressed)
	end
	GUI.IsItemHovered(loadButtonHover)
	GUI.End()
	
	-- Pop the transparent background colors for all buttons
	GUI.PopStyleColor(3)
	
	-- Pop the removal of frame padding and border size
	GUI.PopStyleVar(2)
	
	-- Check if the EXIT button is pressed; close the engine if it is
	if exitButtonPressed:isChecked() then
		ErrHandlerLoc.logErrorType(ErrorType.Info, 'Exit called from ' .. getLuaFilename())
		setEngineRunning(false)
		exitButtonPressed:uncheck()
	end
	
	-- Check if the ABOUT button is pressed
	if aboutButtonPressed:isChecked() then
		sendData(SystemType.GUI, DataType.DataType_AboutWindow, true)
		aboutButtonPressed:uncheck()
	end
	
	-- Check if the OPTIONS button is pressed
	if optionsButtonPressed:isChecked() then
		ErrHandlerLoc.logErrorType(ErrorType.Warning, 'Options window is not implemented yet')
		optionsButtonPressed:uncheck()
	end
	
	-- Check if the EDITOR button is pressed; change the current engine state to EDITOR, if it is
	if editorButtonPressed:isChecked() then
		if not fileBrowserEditMapOpened then
			fileBrowserEditMapOpened = true
			GUI.FileDialog(fileBrowserEditMap)
			editorButtonPressed:uncheck()
		end
		--sendEngineChange(EngineChangeType.StateChange, EngineStateType.Editor)
		--setMouseCapture(true)
	end
	
	-- Check if the PLAY button is pressed; change the current engine state to PLAY, if it is
	if playButtonPressed:isChecked() then
		sendEngineChange(EngineChangeType.StateChange, EngineStateType.Play)
		setMouseCapture(true)
		playButtonPressed:uncheck()
	end
	
	-- Check if the LOAD button is pressed
	if loadButtonPressed:isChecked() then
		if not fileBrowserLoadMapOpened then
			fileBrowserLoadMapOpened = true
			GUI.FileDialog(fileBrowserLoadMap)
			loadButtonPressed:uncheck()
		end
	end
	
	-- Load map file browser
	if fileBrowserLoadMap.m_closed then
		if fileBrowserLoadMap.m_success then
			infoText = 'Loading map: "' .. fileBrowserLoadMap.m_filename .. '"'
			ErrHandlerLoc.logErrorType(ErrorType.Info, infoText)
			
			sendEngineChange(EngineChangeType.SceneFilename, EngineStateType.Play, fileBrowserLoadMap.m_filename)
			sendEngineChange(EngineChangeType.StateChange, EngineStateType.Play)
			
			setMouseCapture(true)
		end
		fileBrowserLoadMapOpened = false
		fileBrowserLoadMap:reset()
	end
	
	-- Edit map file browser
	if fileBrowserEditMap.m_closed then
		if fileBrowserEditMap.m_success then
			infoText = 'Edit map: "' .. fileBrowserEditMap.m_filename .. '"'
			ErrHandlerLoc.logErrorType(ErrorType.Info, infoText)
			
			sendEngineChange(EngineChangeType.SceneFilename, EngineStateType.Editor, fileBrowserEditMap.m_filename)
			sendEngineChange(EngineChangeType.StateChange, EngineStateType.Editor)
			
			setMouseCapture(true)
		end
		fileBrowserEditMapOpened = false
		fileBrowserEditMap:reset()
	end
	
end