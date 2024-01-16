#include "Cloth.h"
#include "ClothPoint.h"
#include "ClothStick.h"

Cloth::Cloth(int width, int height, int spacing, Vector3f start)
	: m_width(width)
	, m_height(height)
	, m_spacing(spacing)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			ClothPoint* point = new ClothPoint(this, Vector3f(start.x + (x * spacing), start.y + (y * spacing), 0));
			
			if (x != 0)
			{
				ClothPoint* leftPoint = m_vPoints[m_vPoints.size() - 1];
				ClothStick* stick = new ClothStick(point, leftPoint);
				leftPoint->AddStick(stick, 0);
				point->AddStick(stick, 0);
				m_vSticks.push_back(stick);
			}
			if (y != 0)
			{
				ClothPoint* upPoint = m_vPoints[x + (y - 1) * (width)];
				ClothStick* stick = new ClothStick(point, upPoint);
				upPoint->AddStick(stick, 1);
				point->AddStick(stick, 1);
				m_vSticks.push_back(stick);
			}

			if (y == 0)
			{
				point->SetIsPinned(true);
			}

			m_vPoints.push_back(point);
		}
	}
}

Cloth::~Cloth()
{
}

void Cloth::Initialise()
{
	CreateCloth();
}


void Cloth::CreateCloth()
{
	const u32 kVertices = m_width * m_height;
	std::vector<u32> colours(kVertices, 0xFFFFffff);


	/// CALCULATE POSITIONS ----------------------------------------------------

	std::vector<Vector3f> positions(kVertices);
	for (u32 i = 0; i < m_height; i++)
	{
		for (u32 j = 0; j < m_width; j++)
		{
			u32 index = (i * m_width) + j;
			positions[index] = Vector3f(j, i, 0);
		}
	}

	/// CALCULATE INDICES ------------------------------------------------------

	std::vector<u32> indices;

	for (int i = 0; i < m_height - 1; i++)
	{
		for (int j = 0; j < m_width - 1; j++)
		{
			u32 squ = (i * m_width) + j;

			indices.push_back(squ);
			indices.push_back(squ + 1);
			indices.push_back(squ + m_width + 1);
			indices.push_back(squ);
			indices.push_back(squ + m_width + 1);
			indices.push_back(squ + m_width);
		}
	}


	/// CALCULATE VERTEX NORMALS -----------------------------------------------

	std::vector<Vector3f> normals(kVertices, Vector3f(0, 0, 0));
	for (int i = 0; i < indices.size(); i+=3)
	{
		Vector3f AB = positions[indices[i + 1]] - positions[indices[i]];
		Vector3f AC = positions[indices[i + 2]] - positions[indices[i]];
		Vector3f fN = cross(AB, AC);
		fN = normalize(fN);

		normals[indices[i]] += fN;
		normals[indices[i + 1]] += fN;
		normals[indices[i + 2]] += fN;
	}

	for (int i = 0; i < normals.size(); i++)
	{
		normals[i] = normalize(normals[i]);
	}


	/// CREATE MESH ------------------------------------------------------------

	Graphics::MeshDesc desc;
	Graphics::StreamInfo streamInfos[5];
	streamInfos[0].m_type = Graphics::StreamType::POSITION;
	streamInfos[0].m_pData = positions.data();
	streamInfos[0].m_dataSize = positions.size() * sizeof(Vector3f);

	streamInfos[1].m_type = Graphics::StreamType::COLOUR;
	streamInfos[1].m_pData = colours.data();
	streamInfos[1].m_dataSize = colours.size() * sizeof(u32);

	streamInfos[2].m_type = Graphics::StreamType::NORMAL;
	streamInfos[2].m_pData = normals.data();
	streamInfos[2].m_dataSize = normals.size() * sizeof(Vector3f);

	streamInfos[3].m_type = Graphics::StreamType::INDEX;
	streamInfos[3].m_pData = indices.data();
	streamInfos[3].m_dataSize = indices.size() * sizeof(u32);

	desc.m_pStreams = streamInfos;
	desc.m_streamCount = 4;
	desc.m_vertexCount = (u32)positions.size();
	desc.m_indexCount = (u32)indices.size();
	
	m_clothMesh = Resources::CreateAsset<Graphics::Mesh>(desc);
}


void Cloth::Update(float dT)
{
	for (ClothPoint* p : m_vPoints)
	{
		p->Update(dT);
	}
	for (ClothStick* s : m_vSticks)
	{
		s->Update();
	}
}

void Cloth::Render()
{
	for (ClothStick* s : m_vSticks)
	{
		s->Render();
	}
}

void Cloth::Destroy()
{
	for (ClothPoint* point : m_vPoints)
	{
		delete point;
	}
	for (ClothStick* stick : m_vSticks)
	{
		delete stick;
	}
}
