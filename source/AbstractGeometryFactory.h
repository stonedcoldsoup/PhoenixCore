/*

Copyright (c) 2009, Jonathan Wayne Parrott

Please see the license.txt file included with this source
distribution for more information.

*/


#ifndef __PH_AB_GF_H__
#define __PH_AB_GF_H__

#include "Texture.h"
#include "BatchGeometry.h"

namespace phoenix
{

//! Abstract Geometry Factory Application Flags
enum E_GEOMFACTORY_FLAGS
{
    EFF_NONE = 0x0000, //!< No properties applied.
    EFF_DEPTH = 0x0001, //!< Depth applied.
    EFF_GROUP = 0x0002, //!< Group applied.
    EFF_TEXTURE = 0x0004, //!< Texture applied.
    EFF_FUNCTIONS = 0x0008, //!< Group Functions applied.
    EFF_UPDATE = 0x0010, //!< Update Called.
    EFF_ALL_NO_UPDATE = 0x000F, //!< All applied but update not called.
    EFF_ALL = 0x001F //!< Alll applied & updated called.
};

//! Abstract Geometry Factory
/*!
    This class provides abstraction of common properties of geometry factories. It just saves us from having
    to write a bunch of accessors and gives a good deal of flexibility to geometry factories.
*/
class AbstractGeometryFactory
{

public:

    //! Constructor
    /*!
        You can optionally pre-set the depth, group and texture of the factory.
    */
    AbstractGeometryFactory( float _d = 0.0f, signed int _g = 0, boost::shared_ptr<Texture> _t = boost::shared_ptr<Texture>() )
        : factory_depth( _d ), factory_group( _g ), factory_texture( _t ), factory_group_begin(), factory_group_end()
    {
    }

    //! Destructor
    /*!
        Does nothing.
    */
    virtual ~AbstractGeometryFactory() {}

    //! Set depth.
    inline void setDepth( float _d = 0.0f ) { factory_depth = _d; }

    //! Get depth.
    inline float getDepth() { return factory_depth; }

    //! Set group.
    inline void setGroup( signed int _g = 0 ) { factory_group = _g; }

    //! Get group,
    inline signed int getGroup() { return factory_group; }

    //! Set texture.
    inline void setTexture( boost::shared_ptr<Texture> _t = boost::shared_ptr<Texture>() ) { factory_texture = _t; }

    //! Get texture.
    inline boost::shared_ptr<Texture> getTexture() { return factory_texture; }

    //! Set group begin function
    /*!
        \note Factories do not have to use this. It is used by apply();
    */
    inline void setGroupBeginFunction( const boost::function< void() >& _f = boost::function< void() >() ) { factory_group_begin = _f; }

    //! Get group begin function
    inline const boost::function< void() >& getGroupBeginFunction() { return factory_group_begin; } 

    //! Set group end function
    /*!
        \note Factories do not have to use this. It is used by apply();
    */
    inline void setGroupEndFunction( const boost::function< void() >& _f = boost::function< void() >() ) { factory_group_end = _f; }

    //! Get group end function
    inline const boost::function< void() >& getGroupEndFunction() { return factory_group_end; } 

    //! Apply
    /*!
        Sets all the given geometry's properties enabled in flags to the factories properties (including group functions) and calls update().
        The _flags parameter is a combination of E_GEOMFACTORY_FLAGS.
    */
    inline void apply( boost::shared_ptr<BatchGeometry> _g, unsigned int _flags = EFF_ALL )
    {
        if( _flags & EFF_DEPTH ) _g->setDepth( factory_depth );
        if( _flags & EFF_GROUP ) _g->setGroup( factory_group );
        if( _flags & EFF_TEXTURE ) _g->setTexture( factory_texture );
        if( _flags & EFF_FUNCTIONS ) _g->setGroupBeginFunction( factory_group_begin );
        if( _flags & EFF_FUNCTIONS ) _g->setGroupEndFunction( factory_group_end );
        if( _flags & EFF_UPDATE ) _g->update();
    }

private:

    float factory_depth;
    signed int factory_group;
    boost::shared_ptr<Texture> factory_texture;
    boost::function< void() > factory_group_begin;
    boost::function< void() > factory_group_end;
};

}

#endif // __PH_AB_GF_H__