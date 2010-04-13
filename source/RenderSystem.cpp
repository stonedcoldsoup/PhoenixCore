/*

Copyright (c) 2010, Jonathan Wayne Parrott

Please see the license.txt file included with this source
distribution for more information.

*/

#include "RenderSystem.h"
#include "PrecompiledFont.h"
#include "BitmapFont.h"
#include "GLFWWindowManager.h"
#include "soil/SOIL.h"

using namespace phoenix;

// Static initialization
RenderSystemPtr phoenix::RenderSystem::instance = RenderSystemPtr();

////////////////////////////////////////////////////////////////////////////////
//Construct & Destruct
////////////////////////////////////////////////////////////////////////////////

RenderSystemPtr RenderSystem::Initialize( const Vector2d& _sz , bool _fs  )
{
	
	// Create the instance
	if( instance ) throw BadInstance();
	instance = RenderSystemPtr( new RenderSystem ); 

	// GLFW Window Manager
	instance->wm = GLFWWindowManager::Instance();

	// Create our window
	if( !instance->wm->open( _sz, _fs ) ) { throw; }

	// Listen to events.
	instance->event_connection = instance->wm->listen( boost::bind( &RenderSystem::onWindowEvent, instance.get(), _1 ) );

    // viewport the same as the window size.
    instance->renderer.getView().setSize();

    // Orthogonal projection.
    glMatrixMode(GL_PROJECTION); 
    glLoadIdentity();
    glOrtho(0.0f, _sz.getX(), _sz.getY(), 0.0f, 1000.0f, -1000.0f);

    // load up identity for the modelview matrix.
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Set up depth buffer
    #ifdef DISABLE_DEPTHBUFFER
        // No need for depth buffer.
        glDisable(GL_DEPTH_TEST);
    #else
        // Enable depth testing and set the function
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
    #endif

    // Smoooth shading, Nicest Hinting.
    glShadeModel(GL_SMOOTH);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

    // Enable blending and set our blending mode.
    glEnable(GL_BLEND);
    instance->setBlendMode(); // Default is for 2d transluceny with RGBA textures.

    //! Default material is white.
    GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    // Material mode
    glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
    glEnable(GL_COLOR_MATERIAL);

    //load our default font
	instance->font = new BitmapFont( instance->resources, instance->renderer , 
		instance->loadTexture(
			 builtin_font_imagedata,
			 builtin_font_imagedata_size,
			 builtin_font_imagedata_width,
			 builtin_font_imagedata_height
		)
	);

    // Clear the screen to black 
    instance->clearScreen( Color(0,0,0) );

    //!start the timer
    instance->fpstimer.start();

	// Setup the debug console
	DebugConsole::Initialize( instance->renderer, instance->font );

	// finally, return the instance.
	return instance;

}


RenderSystem::~RenderSystem()
{
	event_connection.disconnect();
	wm->close();
}


////////////////////////////////////////////////////////////////////////////////
//run function, should be called once every loop, it makes things ready for
//a render.
////////////////////////////////////////////////////////////////////////////////

bool RenderSystem::run()
{

    //Call our own draw function
    renderer.draw();

    //flip the screen (this also polls events).
	wm->update();

    //Clean resources
    resources.clean();

    //clear screen
    clearScreen();

    //store the new framerate
    double newframerate = 1.0f / fpstimer.getTime();
    framerate = (0.6f * newframerate) + (0.4f * framerate);

    //Start our fps timer
    fpstimer.reset();

    return !quit; // Quit is set to true when the window manager has signaled to close.
}

////////////////////////////////////////////////////////////////////////////////
//Load texture function
////////////////////////////////////////////////////////////////////////////////

void RenderSystem::onWindowEvent( const WindowEvent& e )
{
	switch( e.type )
	{
	case WET_CLOSE:
		quit = true;
		break;
	case WET_KEY:
		if( e.bool_data == true && e.int_data == PHK_ESC ){
			quit = true;
		}
		break;
	case WET_RESIZE:
		if( !resize ){
			wm->setWindowSize( renderer.getView().getSize() );
		} else {
			renderer.getView().setSize( e.vector_data );
			glMatrixMode(GL_PROJECTION); 
			glLoadIdentity();
			glOrtho(0.0f, e.vector_data.getX(), e.vector_data.getY(), 0.0f, 1000.0f, -1000.0f);
		}
	default:
		break;
	};
}

////////////////////////////////////////////////////////////////////////////////
//Load texture function
////////////////////////////////////////////////////////////////////////////////

TexturePtr RenderSystem::loadTexture( const std::string& _fn, bool _l )
{

	//This is the class that will hold our texture
	TexturePtr ctext = new Texture( resources );

	int width=0,height=0;

	GLuint newtextid = SOIL_load_OGL_texture
        (
                _fn.c_str(),
                SOIL_LOAD_RGBA,
                SOIL_CREATE_NEW_ID,
                SOIL_FLAG_TEXTURE_REPEATS,
                &width, &height
        );

	if( newtextid != 0 )
	{

        //Load the texture
        glBindTexture(GL_TEXTURE_2D, newtextid);

        //use linear filtering
        if ( _l == true)
        {

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        }
        else
        {

            //Use nearest filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        }

        //Set up the Texture class
        ctext->setTextureId(newtextid);
        ctext->setWidth(width);
        ctext->setHeight(height);
        ctext->setName( _fn );

        //Return our texture
        return ctext;

    }
    else
    {

        TexturePtr ctext = new Texture( resources );
        ctext->setTextureId(0);
        ctext->setWidth(0);
        ctext->setHeight(0);
        ctext->setName("FAILED TO LOAD");

        //Return our texture
        return ctext;

    }

    //should never happen
    return TexturePtr();
    assert(true);

}

// Load texture from memory.
TexturePtr RenderSystem::loadTexture( const unsigned char* const _d, const unsigned int _len, const unsigned int _w, const unsigned int _h, bool _lin )
{

	//This is the class that will hold our texture
	TexturePtr ctext = new Texture( resources );

	GLuint newtextid = SOIL_load_OGL_texture_from_memory(
		_d,
		_len,
		SOIL_LOAD_RGBA,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_TEXTURE_REPEATS
	);

	if( newtextid != 0 )
	{
        //Load the texture
        glBindTexture(GL_TEXTURE_2D, newtextid);

        //use linear filtering
        if ( _lin == true)
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
        else
        {
            //Use nearest filter
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }

        //Set up the Texture class
        ctext->setTextureId(newtextid);
        ctext->setWidth( _w );
        ctext->setHeight( _h );
        ctext->setName( "Loaded From Memory" );

        //Return our texture
        return ctext;
    }
    else
    {
        TexturePtr ctext = new Texture( resources );
        ctext->setTextureId(0);
        ctext->setWidth(0);
        ctext->setHeight(0);
        ctext->setName("FAILED TO LOAD");

        //Return our texture
        return ctext;
    }

    //should never happen
    return TexturePtr();
    assert(true);

}


////////////////////////////////////////////////////////////////////////////////
// Find texture functions.
////////////////////////////////////////////////////////////////////////////////

//! Find texture.
TexturePtr RenderSystem::findTexture(const std::string& _i)
{
    ResourcePtr findtexture = resources.find(_i);
    if( findtexture )
    {
        return findtexture->grab< Texture >();
    }
    else
    {
        return TexturePtr();
    }

}

//! Find texture.
TexturePtr RenderSystem::findTexture(const GLuint& _n)
{
	boost::recursive_mutex::scoped_lock( resources.getMutex() );
    for (unsigned int i=0;i<resources.count();i++)
    {
        if( resources.get(i)->getType() == ERT_TEXTURE )
        {
            if ( resources.get(i)->grab<Texture>()->getTextureId() == _n )
            {
                return resources.get(i)->grab<Texture>();
            }
        }
    }
    return TexturePtr();
}


////////////////////////////////////////////////////////////////////////////////
//Draw text on the screen
////////////////////////////////////////////////////////////////////////////////

BatchGeometryPtr RenderSystem::drawText( const std::string& _s, const Vector2d& _p, const Color& _c)
{
	font->setColor( _c );
    font->setDepth( factory.getDepth() );
    font->setGroup( factory.getGroup() );
	return font->drawText( _s, _p );
}

