#include"objloader.hpp"

namespace vcl{

namespace obj{

static inline bool operator < ( const vec3i &a, const vec3i &b) {
    if (a.i != b.i) return(a.i < b.i);
    if (a.j != b.j) return(a.j < b.j);
    if (a.k != b.k) return(a.k < b.k);
    return(false);
}

objloader::objloader( const std::string& _path, FILE *objFile ) : path( _path ) {

    /*! bookkeeping buffer */
    std::vector< std::vector< vec3i > > faceGroup;
	//std::map< vec3i, uint32_t > vertexMap;
	size_t offset = 0;

    /*! current mesh material name */
    char materialName[ 1024 ];  sprintf(materialName, "default");

    /*! iterate over lines of the file, flush the face group on EOF */
    for (char line[1024] ; fgets(line, 1024, objFile) ? true : (flushFaceGroup(faceGroup, materialName, offset), false); ) {
	//for (char line[1024] ; fgets(line, 1024, objFile) ? true : (flushFaceGroup(faceGroup, materialName, vertexMap, offset), false); ) {
		
        /*! acquire the first token on this line */
		char token[1024];  if (sscanf(line, "%s", token) == EOF) continue;

        /*! face definition */
        if (!strcmp(token, "f")) { std::vector< vec3i > face;  loadFace(line, face);  faceGroup.push_back(face); }

        /*! load material library */
        if (!strcmp(token, "mtllib")) {
            char libraryName[1024];  sscanf(line, "%*s %s", libraryName);  loadMTL( path + libraryName );
        }

        /*! use material */
        if (!strcmp(token, "usemtl")) { flushFaceGroup(faceGroup, materialName, offset);  sscanf(line, "%*s %s", materialName); }
		//if (!strcmp(token, "usemtl")) { flushFaceGroup(faceGroup, materialName, vertexMap, offset);  sscanf(line, "%*s %s", materialName); }

        /*! vertex coordinates */
        if (!strcmp(token, "v"))  { vec3f value;  sscanf(line, "%*s %f %f %f", &value.x, &value.y, &value.z);  v.push_back(value); }

        /*! vertex normal */
        if (!strcmp(token, "vn")) { vec3f value;  sscanf(line, "%*s %f %f %f", &value.x, &value.y, &value.z);  vn.push_back(value); }

        /*! texture coordinates */
        if (!strcmp(token, "vt")) { vec2f value;  sscanf(line, "%*s %f %f", &value.x, &value.y);  vt.push_back(value); }

    }

}

//uint32_t objloader::appendVertex( const vec3i &vertex, mesh& _mesh, std::map<vec3i, uint32_t> &vertexMap) {
uint32_t objloader::appendVertex( const vec3i &vertex, mesh& _mesh, std::map<vec3i, uint32_t> &vertexMap, size_t &offset) {

    /*! determine if we've seen this vertex before */
    const std::map<vec3i, uint32_t>::iterator &entry = vertexMap.find(vertex);

    /*! two vertices match only if positions, normals, and texture coordinates match */
    if (entry != vertexMap.end()) return(entry->second);

    /*! this is a new vertex, store the indices */
    if ( vertex.i >= 0 ) _mesh.positions.push_back(v[vertex.i]);
    if ( vertex.j >= 0 ) _mesh.normals.push_back(vn[vertex.j]);
    if ( vertex.k >= 0 ) _mesh.texcoords.push_back(vt[vertex.k]);

    /*! map this vertex to a unique id */
    return(vertexMap[vertex] = int( _mesh.positions.size() + offset) - 1);

}

//void objloader::flushFaceGroup( std::vector<std::vector< vec3i > > &faceGroup, const std::string materialName) {
//void objloader::flushFaceGroup( std::vector< std::vector< vec3i > > &faceGroup, const std::string materialName, std::map< vec3i, uint32_t > &vertexMap, size_t &offset ){
void objloader::flushFaceGroup( std::vector< std::vector< vec3i > > &faceGroup, const std::string materialName, size_t &offset ){

    /*! temporary storage */
    std::map< vec3i, uint32_t > vertexMap;

    /*! mesh that will be constructed from this face group */
    mesh _mesh;  _mesh.material = materials[materialName];

    /*! construct a mesh for this face group */
    for (size_t face=0 ; face < faceGroup.size() ; face++) {

        /*! triangulate the face with a triangle fan */
        for (size_t i=0, j=1, k=2 ; k < faceGroup[face].size() ; j++, k++) {
            vec3i triangle;
            triangle.i = appendVertex(faceGroup[face][i], _mesh, vertexMap, offset);
            triangle.j = appendVertex(faceGroup[face][j], _mesh, vertexMap, offset);
            triangle.k = appendVertex(faceGroup[face][k], _mesh, vertexMap, offset);
            _mesh.triangles.push_back( triangle );
        }
    }

    /* fix some materials */
    if (//mesh.material.name == "BeltStrap1" ||
        //mesh.material.name == "BeltBuckle1" ||
        //mesh.material.name == "Belt1" ||
            _mesh.material.name == "JeansStraps1" ||
            //mesh.material.name == "JeansButton1" ||
            _mesh.material.name == "Jeans1")// ||
        //mesh.material.name == "2_SkinHip")
    {
        _mesh.material.Ks.r = 0; _mesh.material.Ks.g = 0; _mesh.material.Ks.b = 0;
    }

    /* fix some texture coordinate issues */
    for (size_t i=0; i < _mesh.texcoords.size(); i++)
        _mesh.texcoords[i].y = 1.0f - _mesh.texcoords[i].y;

    /*! append the mesh to the model */
    if (faceGroup.size()) model.push_back( _mesh );
    faceGroup.clear();
	offset += _mesh.positions.size();
}

void objloader::flushMaterial( material& _material, const std::string materialName ) {
    if ( strstr( _material.name.c_str(),"Skin") ) {
        _material.Ks.r = 1.0f; _material.Ks.g = 1.0f; _material.Ks.b = 1.0f;
    }
    /*! store the material */
    materials[materialName] = _material;
    /*! clear the material */
    _material = material();
}

void objloader::loadFace(char *line, std::vector< vec3i > &face) {

    for (char *token = strtok(line, " f\t\r\n") ; token ; token = strtok(NULL, " \t\r\n")) {

        /*! vertex is defined as indices into position, normal, texture coordinate buffers */
        vec3i vertex;  vertex.i = -1, vertex.j = -1, vertex.k = -1;

        /*! vertex has texture coordinates and a normal */
        if (sscanf(token, "%d/%d/%d", &vertex.i, &vertex.k, &vertex.j) == 3) {

            vertex.i = (vertex.i > 0) ? vertex.i - 1 : (vertex.i == 0 ? 0 :  v.size() + vertex.i);
            vertex.j = (vertex.j > 0) ? vertex.j - 1 : (vertex.j == 0 ? 0 : vn.size() + vertex.j);
            vertex.k = (vertex.k > 0) ? vertex.k - 1 : (vertex.k == 0 ? 0 : vt.size() + vertex.k);
            face.push_back(vertex);

            /*! vertex has a normal */
        } else if (sscanf(token, "%d//%d", &vertex.i, &vertex.j) == 2) {

            vertex.i = (vertex.i > 0) ? vertex.i - 1 : (vertex.i == 0 ? 0 :  v.size() + vertex.i);
            vertex.j = (vertex.j > 0) ? vertex.j - 1 : (vertex.j == 0 ? 0 : vn.size() + vertex.j);
            face.push_back(vertex);

            /*! vertex has texture coordinates */
        } else if (sscanf(token, "%d/%d", &vertex.i, &vertex.k) == 2) {

            vertex.i = (vertex.i > 0) ? vertex.i - 1 : (vertex.i == 0 ? 0 :  v.size() + vertex.i);
            vertex.k = (vertex.k > 0) ? vertex.k - 1 : (vertex.k == 0 ? 0 : vt.size() + vertex.k);
            face.push_back(vertex);

            /*! vertex has no texture coordinates or normal */
        } else if (sscanf(token, "%d", &vertex.i) == 1) {

            vertex.i = (vertex.i > 0) ? vertex.i - 1 : (vertex.i == 0 ? 0 : v.size() + vertex.i);
            face.push_back(vertex);
        }
    }
}

char* objloader::parseString( const char* in, char* out )
{
    in+=strspn(in, " \t");
    if (in[0] == '\"') in++;
    strcpy(out,in);
    while (true) {
        size_t len = strlen(out);
        if (len == 0) return NULL;
        if (out[len-1] != '\"' && out[len-1] != ' ' && out[len-1] != '\r' && out[len-1] != '\n') break;
        out[len-1] = 0;
    }

    if (out[0] == '/') out++;
    const char* apath = "C:/Users/swoop/Documents/DAZ 3D/Studio/My Library/Runtime/";
    if (strstr(out,apath) == out) {
        out+=strlen(apath);
        if (out[0] == 'T') out[0] = 't';
    }
    return out;
}

void objloader::loadMTL( const std::string libraryName ) {

    /*! open the MTL file */
    FILE *mtlFile = fopen(libraryName.c_str(), "r" );
    if (!mtlFile) { printf("  ERROR:  unable to open %s\n", libraryName.c_str());  return; }

    /*! current material and name */
    material _material;  char materialName[1024];  sprintf(materialName, "default");

    /*! iterate over lines of the file, store the current material on EOF */
    for (char line[1024] ; fgets(line, 1024, mtlFile) ? true : (flushMaterial( _material, materialName ), false); ) {

        /*! acquire the first token on this line */
		char token[1024];  if (sscanf(line, "%s", token) == EOF) continue;
		
        /*! ignore comments */
        if (!strcmp(token, "#")) continue;

        /*! opacity value */
        if (!strcmp(token, "d")) { sscanf(line, "%*s %f", &_material.d); }

        /*! ambient color */
        if (!strcmp(token, "Ka")) { sscanf(line, "%*s %f %f %f", &_material.Ka.r, &_material.Ka.g, &_material.Ka.b); }

        /*! diffuse color */
        if (!strcmp(token, "Kd")) { sscanf(line, "%*s %f %f %f", &_material.Kd.r, &_material.Kd.g, &_material.Kd.b); }

        /*! specular color */
        if (!strcmp(token, "Ks")) { sscanf(line, "%*s %f %f %f", &_material.Ks.r, &_material.Ks.g, &_material.Ks.b); }

        /*! opacity texture */
        if (!strcmp(token, "map_d")) { char textureName[1024];  _material.map_d = parseString(line+5,textureName); }

        /*! ambient color texture */
        if (!strcmp(token, "map_Ka")) { char textureName[1024];  _material.map_Ka = parseString(line+6,textureName); }

        /*! diffuse color texture */
        if (!strcmp(token, "map_Kd")) { char textureName[1024];  _material.map_Kd = parseString(line+6,textureName); }

        /*! specular color texture */
        if (!strcmp(token, "map_Ks")) { char textureName[1024];  _material.map_Ks = parseString(line+6,textureName); }

        /*! specular coefficient texture */
        if (!strcmp(token, "map_Ns")) { char textureName[1024];  _material.map_Ns = parseString(line+6,textureName); }

        /*! bump map */
        if (!strcmp(token, "map_Bump")) { char textureName[1024];  _material.map_Bump = parseString(line+8,textureName); }

        /*! new material delimiter */
        if (!strcmp(token, "newmtl")) { flushMaterial( _material, materialName);  sscanf(line, "%*s %s", materialName); _material.name = materialName; }

        /*! specular coefficient */
        if (!strcmp(token, "Ns")) { sscanf(line, "%*s %f", &_material.Ns); }

    } fclose(mtlFile);
}

}
}