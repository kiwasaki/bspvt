// ======================================================================== //
// Copyright 2009-2013 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#ifndef __OBJ_LOADER_H__
#define __OBJ_LOADER_H__

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>

namespace vcl {

namespace obj{

    struct col3f {
        col3f() : r( 0.f ), g( 0.f ), b( 0.f ) {}
        col3f( const float _r, const float _g, const float _b ) : r( _r ), g( _g ), b( _b ) {}
        float r, g, b;
    };
    struct vec2f {
        float x, y;
    };
    struct vec3f {
        float x, y, z;
    };
    struct vec3i {
        int i, j, k;
    };

    struct material {
        std::string name;
        std::string map_d;
        float d; //opacity value
        std::string map_Ka;
        col3f Ka; //ambient color
        std::string map_Kd;
        col3f Kd; //diffuse color
        std::string map_Ks;
        col3f Ks; //specular color
        std::string map_Ns;
        float Ns; //specular coefficient
        std::string map_Bump; //bump map filename

        material() : Ka( 0.5f, 0.5f, 0.5f ), d( 1.f ), Kd( 0.5f, 0.5f, 0.5f ), Ns( 0.f ), Ks( 0.0f, 0.0f, 0.0f ) {}
    };

    struct mesh {
        std::vector< vec3f > positions;
        std::vector< vec3f > normals;
        std::vector< vec2f > texcoords;
        std::vector< vec3i > triangles;
        obj::material material;
    };


    class objloader {

    public:
        /*! loaded geometry */
        std::vector< mesh > model;

        objloader( const std::string &_path, FILE *objFile );

    private:

        std::string path;

        /*! bookkeeping buffers */
        std::vector< vec3f > v;
        std::vector< vec3f > vn;
        std::vector< vec2f > vt;

        /*! materials library */
        std::map< std::string, material > materials;

        /*! parses a string */
        char *parseString( const char *in, char *out );

        /*! append unique vertices to the mesh */
        uint32_t appendVertex( const vec3i &vertex, mesh &mesh, std::map< vec3i, uint32_t > &vertexMap );
		uint32_t appendVertex( const vec3i &vertex, mesh &mesh, std::map< vec3i, uint32_t > &vertexMap, size_t &offset );

        /*! write out the face list defining the current mesh */
        void flushFaceGroup( std::vector< std::vector< vec3i > > &faceGroup, const std::string materialName );
		void flushFaceGroup( std::vector< std::vector< vec3i > > &faceGroup, const std::string materialName, std::map< vec3i, uint32_t > &vertexMap, size_t &offset );
		void flushFaceGroup( std::vector< std::vector< vec3i > > &faceGroup, const std::string materialName, size_t &offset );

        /*! store and reset the current material */
        void flushMaterial( material &material, const std::string materialName );

        /*! load attribute buffer indices for each vertex in the current face */
        void loadFace( char *line, std::vector< vec3i > &face );

        /*! load an OBJ material library */
        void loadMTL( const std::string libraryName );

    };

    inline std::vector< mesh > loadobj( const char *fileName )
    {
        const std::string path = "";
        /*! open the OBJ file */
        FILE *objFile = fopen( fileName, "r" );
        if ( !objFile ) printf( "  ERROR:  unable to open %s\n", fileName ), exit( 1 );

        /*! the main loader routine */
        objloader loader( path, objFile );
        fclose( objFile );
        return ( loader.model );
    }

    inline std::vector< mesh > loadobj( const std::string &_path, const std::string &_filename )
    {
        const std::string filename = _path + _filename;
        FILE *objFile = fopen( filename.c_str(), "r" );
        if ( !objFile ) std::cerr << " ERROR : unable to open " << filename << "\n", exit( 1 );
        objloader loader( _path, objFile );
        fclose( objFile );
        return ( loader.model );
    }
};

};

#include "objloader-impl.hpp"

#endif
