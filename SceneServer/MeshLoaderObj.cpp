//
// Copyright (c) 2009-2010 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#include "MeshLoaderObj.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "xLog.h"
#include "xJsonParser.h"
#include "xLuaTable.h"

rcMeshLoaderObj::rcMeshLoaderObj() :
  m_scale(1.0f),
  m_verts(0),
  m_tris(0),
  m_normals(0),
  m_vertCount(0),
  m_triCount(0)
{
  bzero(m_filename, sizeof(m_filename));
}

rcMeshLoaderObj::~rcMeshLoaderObj()
{
  SAFE_DELETE_VEC(m_verts);
  SAFE_DELETE_VEC(m_normals);
  SAFE_DELETE_VEC(m_tris);
}

void rcMeshLoaderObj::addVertex(float x, float y, float z, int& cap)
{
  if (m_vertCount+1 > cap)
  {
    cap = !cap ? 8 : cap*2;
    float* nv = NEW float[cap*3];
    if (m_vertCount)
      memcpy(nv, m_verts, m_vertCount*3*sizeof(float));
    SAFE_DELETE_VEC(m_verts);
    m_verts = nv;
  }
  float* dst = &m_verts[m_vertCount*3];
  *dst++ = x*m_scale;
  *dst++ = y*m_scale;
  *dst++ = z*m_scale;
  m_vertCount++;
 // XLOG("[Vertex],%f,%f,%f,count:%d,cap:%d", x*m_scale, y*m_scale, z*m_scale, m_vertCount, cap);
}

void rcMeshLoaderObj::addTriangle(int a, int b, int c, int& cap)
{
  if (m_triCount+1 > cap)
  {
    cap = !cap ? 8 : cap*2;
    int* nv = NEW int[cap*3];
    if (m_triCount)
      memcpy(nv, m_tris, m_triCount*3*sizeof(int));
    SAFE_DELETE_VEC(m_tris);
    m_tris = nv;
  }
  int* dst = &m_tris[m_triCount*3];
  *dst++ = a;
  *dst++ = b;
  *dst++ = c;
  m_triCount++;
  //XLOG("[Triangle],%d,%d,%d,count:%u", a, b, c, m_triCount);
}

static char* parseRow(char* buf, char* bufEnd, char* row, int len)
{
  bool start = true;
  bool done = false;
  int n = 0;
  while (!done && buf < bufEnd)
  {
    char c = *buf;
    buf++;
    // multirow
    switch (c)
    {
      case '\\':
        break;
      case '\n':
        if (start) break;
        done = true;
        break;
      case '\r':
        break;
      case '\t':
      case ' ':
        if (start) break;
      default:
        start = false;
        row[n++] = c;
        if (n >= len-1)
          done = true;
        break;
    }
  }
  row[n] = '\0';
  return buf;
}

static int parseFace(char* row, int* data, int n, int vcnt)
{
  int j = 0;
  while (*row != '\0')
  {
    // Skip initial white space
    while (*row != '\0' && (*row == ' ' || *row == '\t'))
      row++;
    char* s = row;
    // Find vertex delimiter and terminated the string there for conversion.
    while (*row != '\0' && *row != ' ' && *row != '\t')
    {
      if (*row == '/') *row = '\0';
      row++;
    }
    if (*s == '\0')
      continue;
    int vi = atoi(s);
    data[j++] = vi < 0 ? vi+vcnt : vi-1;
    if (j >= n) return j;
  }
  return j;
}

bool rcMeshLoaderObj::load(const char* filename)
{
  char* buf = 0;
  FILE* fp = fopen(filename, "rb");
  if (!fp)
    return false;
  fseek(fp, 0, SEEK_END);
  int bufSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  buf = NEW char[bufSize];
  if (!buf)
  {
    fclose(fp);
    return false;
  }
  size_t readLen = fread(buf, bufSize, 1, fp);
  fclose(fp);

  if (readLen != 1)
  {
    delete[] buf;
    return false;
  }

  char* src = buf;
  char* srcEnd = buf + bufSize;
  char row[512];
  int face[32];
  float x,y,z;
  int nv;
  int vcap = 0;
  int tcap = 0;

  while (src < srcEnd)
  {
    // Parse one row
    row[0] = '\0';
    src = parseRow(src, srcEnd, row, sizeof(row)/sizeof(char));
    // Skip comments
    if (row[0] == '#') continue;
    if (row[0] == 'v' && row[1] != 'n' && row[1] != 't')
    {
      // Vertex pos
      sscanf(row+1, "%f %f %f", &x, &y, &z);
      addVertex(x, y, z, vcap);
    }
    if (row[0] == 'f')
    {
      // Faces
      nv = parseFace(row+1, face, 32, m_vertCount);
      for (int i = 2; i < nv; ++i)
      {
        const int a = face[0];
        const int b = face[i-1];
        const int c = face[i];
        if (a < 0 || a >= m_vertCount || b < 0 || b >= m_vertCount || c < 0 || c >= m_vertCount)
          continue;
        addTriangle(a, b, c, tcap);
      }
    }
  }

  SAFE_DELETE_VEC(buf);

  // Calculate normals.
  m_normals = NEW float[m_triCount*3];
  for (int i = 0; i < m_triCount*3; i += 3)
  {
    const float* v0 = &m_verts[m_tris[i]*3];
    const float* v1 = &m_verts[m_tris[i+1]*3];
    const float* v2 = &m_verts[m_tris[i+2]*3];
    float e0[3], e1[3];
    for (int j = 0; j < 3; ++j)
    {
      e0[j] = v1[j] - v0[j];
      e1[j] = v2[j] - v0[j];
    }
    float* n = &m_normals[i];
    n[0] = e0[1]*e1[2] - e0[2]*e1[1];
    n[1] = e0[2]*e1[0] - e0[0]*e1[2];
    n[2] = e0[0]*e1[1] - e0[1]*e1[0];
    float d = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
    if (d > 0)
    {
      d = 1.0f/d;
      n[0] *= d;
      n[1] *= d;
      n[2] *= d;
    }
  }

  strncpy(m_filename, filename, sizeof(m_filename));
  m_filename[sizeof(m_filename)-1] = '\0';

  return true;
}

bool rcMeshLoaderObj::loadJson(const xLuaData &mesh)
{
 // float x,y,z;
  int vcap = 0;
  int tcap = 0;

  int i = 0;
  std::map<DWORD, xLuaData> tmp;
  const xLuaData &vertices = mesh.getData("vertices");
  if (vertices.m_table.empty()) return false;

  for (auto it=vertices.m_table.begin(); it!=vertices.m_table.end(); ++it)
  {
    tmp[atoi(it->first.c_str())] = it->second;
  }

  for (auto it : tmp)
  {
    const xLuaData &item = it.second;
    addVertex(item.getTableFloat("x"), item.getTableFloat("y"), item.getTableFloat("z"), vcap);
    //addVertex(item.getTableFloat("x"), 0.0f, item.getTableFloat("z"), vcap);
    //XLOG("[加载Mesh],vertices:%u,(%f,%f,%f)", it.first, item.getTableFloat("x"), item.getTableFloat("y"), item.getTableFloat("z")); 
  }

  i = 0;
  int triangle[3];
  tmp.clear();
  const xLuaData &indices = mesh.getData("indices");
  for (auto it=indices.m_table.begin(); it!=indices.m_table.end(); ++it)
  {
    tmp[atoi(it->first.c_str())] = it->second;
  }

  for (auto it : tmp)
  {
    triangle[i++] = it.second.getInt();
    if (i>=3)
    {
      addTriangle(triangle[0], triangle[1], triangle[2], tcap);
      i = 0;
   //   XLOG("[加载Mesh],triangle:%u,(%d,%d,%d)", it.first, triangle[0], triangle[1], triangle[2]); 
    }
  }

  // Calculate normals.
  m_normals = NEW float[m_triCount*3];
  for (int i = 0; i < m_triCount*3; i += 3)
  {
    const float* v0 = &m_verts[m_tris[i]*3];
    const float* v1 = &m_verts[m_tris[i+1]*3];
    const float* v2 = &m_verts[m_tris[i+2]*3];
    float e0[3], e1[3];
    for (int j = 0; j < 3; ++j)
    {
      e0[j] = v1[j] - v0[j];
      e1[j] = v2[j] - v0[j];
    }
    float* n = &m_normals[i];
    n[0] = e0[1]*e1[2] - e0[2]*e1[1];
    n[1] = e0[2]*e1[0] - e0[0]*e1[2];
    n[2] = e0[0]*e1[1] - e0[1]*e1[0];
    float d = sqrtf(n[0]*n[0] + n[1]*n[1] + n[2]*n[2]);
    if (d > 0)
    {
      d = 1.0f/d;
      n[0] *= d;
      n[1] *= d;
      n[2] *= d;
    }
  }

  return true;
}
