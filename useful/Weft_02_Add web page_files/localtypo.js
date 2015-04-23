var ToolBar_Supported = ToolBar_Supported ;
if (ToolBar_Supported != null && ToolBar_Supported == true)
{
	//To Turn on/off Frame support, set Frame_Supported = true/false.
	Frame_Supported = false;

	// Customize default ICP menu color - bgColor, fontColor, mouseoverColor
	setDefaultICPMenuColor("#FFFFFF", "white", "red");

	// Customize toolbar background color
	setToolbarBGColor("white");

	// display ICP Banner
	setICPBanner("/typography/graphics/pixel.gif","/typography/","typography Home") ;

}

	
