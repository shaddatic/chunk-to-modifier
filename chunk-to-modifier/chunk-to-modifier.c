#include <chunk-to-modifier.h>

#include <stdbool.h>

#define MOD_CONV_BUFSIZE 512

static char FileName[260];
static int	nbModels;

static int
GetAndUnravelCnkStrips(NJS_CNK_MODEL* pModel, int* pIndexBuffer)
{
	int nbindexes = 0;

	sint16* plist = pModel->plist;
	int type;

	bool hasstrip = false;

	while (1)
	{
		while (1)
		{
			while (1)
			{
				while (1)
				{
					while (1)
					{
						while (1)
						{
							type = ((uint8*)plist)[0];

							if (type >= NJD_BITSOFF)
								break;

							++plist;
						}

						if (type >= NJD_TINYOFF)
							break;

						++plist;
					}

					if (type >= NJD_MATOFF)
						break;

					plist += 2;
				}

				if (type >= NJD_VERTOFF)
					break;

				plist += 2;

				switch (type) {
				case NJD_CM_D:
				case NJD_CM_A:
				case NJD_CM_S:
					plist += 2;
					break;
				case NJD_CM_DA:
				case NJD_CM_DS:
				case NJD_CM_AS:
					plist += 4;
					break;
				case NJD_CM_DAS:
					plist += 6;
					break;
				}
			}

			if (type >= NJD_STRIPOFF)
				break;
		}

		if (type == NJD_ENDOFF)
			break;

		hasstrip = true;

		int nbstrip = plist[2] & 0x3FFF;

		plist += 3;

		while (nbstrip--)
		{
			int len = *plist;

			++plist;

			bool flip = false;

			if (len < 0) /* R */
			{
				len = -len;
				flip = true;
			}

			uint16 stripbuf[MOD_CONV_BUFSIZE];
			int nbstripbuf = 0;

			while (len--)
			{
				stripbuf[nbstripbuf++] = *(uint16*) plist;

				plist += 3;
			}

			for (int i = 0; i < (nbstripbuf - 2); ++i)
			{
				if (flip)
				{
					pIndexBuffer[nbindexes + 0] = stripbuf[i + 2];
					pIndexBuffer[nbindexes + 1] = stripbuf[i + 1];
					pIndexBuffer[nbindexes + 2] = stripbuf[i + 0];
				}
				else
				{
					pIndexBuffer[nbindexes + 0] = stripbuf[i + 0];
					pIndexBuffer[nbindexes + 1] = stripbuf[i + 1];
					pIndexBuffer[nbindexes + 2] = stripbuf[i + 2];
				}

				flip = !flip;
				nbindexes += 3;
			}
		}
	}

	return hasstrip ? nbindexes : -1;
}

static int
GetVListStrideSize(sint32* pVList)
{
	switch (((uint16*) pVList)[0] & 63) {
	case NJD_CV_SH:
	case NJD_CV_D8:
	case NJD_CV_UF:
	case NJD_CV_NF:
	case NJD_CV_S5:
	case NJD_CV_S4:
	case NJD_CV_IN:
	case NJD_CV_VNX:
		return 4;

	case NJD_CV_VN_SH:
		return 8;

	case NJD_CV:
		return 3;

	case NJD_CV_VN:
		return 6;

	case NJD_CV_VN_D8:
	case NJD_CV_VN_UF:
	case NJD_CV_VN_NF:
	case NJD_CV_VN_S5:
	case NJD_CV_VN_S4:
	case NJD_CV_VN_IN:
		return 7;

	case NJD_CV_VNX_D8:
	case NJD_CV_VNX_UF:
		return 5;

	default:
		return -1; 
	}
}

static int
GetVListIdxNum(sint32* pVList)
{
	return ((uint16*)pVList)[3];
}

static void
PrintModPList(const char* name, const int* pIndexBuffer, int nbIndexBuffer)
{
	printf(
		"PLIST      volume_%s[]\n"
		"START\n"
		"    CnkO_P3(), %i, _NB( UFO_0, %i ),\n",

		name,
		(nbIndexBuffer + 2),
		(nbIndexBuffer / 3)
	);

	for (int i = 0; i < nbIndexBuffer; i += 3)
	{
		printf("    %i, %i, %i,\n", 

			pIndexBuffer[i + 0], 
			pIndexBuffer[i + 1], 
			pIndexBuffer[i + 2]
		);
	}

	printf(
		"    CnkNull(),\n"
		"    CnkEnd()\n"
		"END\n\n"
	);
}

static void
PrintModVList(const char* name, const sint32* pVtxList, int nbVtx, int nbVtxStride)
{
	printf(
		"VLIST      vertex_%s[]\n"
		"START\n"
		"    CnkV(0, %i),\n"
		"    OffnbIdx(0, %i),\n", 

		name,
		((nbVtx * 3) + 1),
		nbVtx
	);

	for (int i = 0; i < nbVtx; ++i)
	{
		printf("    VERT( 0x%0*x, 0x%0*x, 0x%0*x ),\n", 

			8, pVtxList[(i * nbVtxStride) + 0], 
			8, pVtxList[(i * nbVtxStride) + 1], 
			8, pVtxList[(i * nbVtxStride) + 2]
		);
	}

	printf(
		"    CnkEnd()\n"
		"END\n\n"
	);
}

static void
PrintModModelSub(const char* name, const NJS_CNK_MODEL* pModel)
{
	printf(
		"CNKMODEL   model_%s[]\n"
		"START\n"
		"VList      vertex_%s,\n"
		"PList      volume_%s,\n"
		"Center     %fF, %fF, %fF,\n"
		"Radius     %fF,\n"
		"END\n\n",

		name,
		name,
		name,
		pModel->center.x, pModel->center.y, pModel->center.z,
		pModel->r
	);
}

static bool
PrintModModel(const char* name, const NJS_CNK_MODEL* pModel)
{
	/* Get PList info */

	int plistbuffer[MOD_CONV_BUFSIZE];

	const int nbplist = GetAndUnravelCnkStrips(pModel, plistbuffer);

	if (nbplist == -1)
		return false;

	/* Get VList info */

	const int vtxstride =	GetVListStrideSize(pModel->vlist);
	const int nbvertex =	GetVListIdxNum(pModel->vlist);

	if (vtxstride == -1 || nbvertex <= 0)
		return false;

	int* vtxlist = pModel->vlist + 2;

	/* Print info */

	PrintModPList(name, plistbuffer, nbplist);
	PrintModVList(name, vtxlist, nbvertex, vtxstride);

	PrintModModelSub(name, pModel);

	return true;
}        

#define AngToDeg(ang) (float) ang / (65536.0F/360.0000F)

static void
PrintModObjectSub(char* pNameBuf, const NJS_CNK_OBJECT* pObject, bool hasModel, int nbChild, int nbSibling)
{
	printf(
		"CNKOBJECT  object_%s[]\n"
		"START\n"
		"EvalFlags  ( 0x%0*x ),\n", 
		
		pNameBuf,
		8, pObject->evalflags
	);

	if (hasModel)
		printf("CNKModel   model_%s,\n", pNameBuf);
	else
		printf("CNKModel   NULL,\n");

	printf(
		"OPosition  (  %fF,  %fF,  %fF ),\n"
		"OAngle     (  %fF,  %fF,  %fF ),\n"
		"OScale     (  %fF,  %fF,  %fF ),\n", 

		pObject->pos[0], pObject->pos[1], pObject->pos[2],
		AngToDeg(pObject->ang[0]), AngToDeg(pObject->ang[1]), AngToDeg(pObject->ang[2]),
		pObject->scl[0], pObject->scl[1], pObject->scl[2]
	);

	if (nbChild != -1)
	{
		snprintf(pNameBuf, 260, "%s_%i", FileName, nbChild);
		printf("Child       object_%s,\n", pNameBuf);
	}
	else
	{
		printf("Child       NULL,\n");
	}

	if (nbSibling != -1)
	{
		snprintf(pNameBuf, 260, "%s_%i", FileName, nbSibling);
		printf("Sibling     object_%s,\n", pNameBuf);
	}
	else
	{
		printf("Sibling     NULL,\n");
	}
	
	printf("END\n\n");
}

static void
PrintModObject(const NJS_CNK_OBJECT* pObject, int nbModel, int nbChild, int nbSibling)
{
	/* Make Object name */

	char objname[260];

	if (nbModel)
		snprintf(objname, 260, "%s_%i", FileName, nbModel);
	else
		snprintf(objname, 260, "%s", FileName);

	/* Print Model if exists */

	bool hasmdl = pObject->model ? PrintModModel(objname, pObject->model) : false;

	/* Print Object */

	PrintModObjectSub(objname, pObject, hasmdl, nbChild, nbSibling);
}

static int
PrintModObjectTree(const NJS_CNK_OBJECT* pObject)
{
	const int nbmodel = nbModels++;

	int nbchild =	pObject->child		? PrintModObjectTree(pObject->child)	: -1;
	int nbsibling = pObject->sibling	? PrintModObjectTree(pObject->sibling)	: -1;

	PrintModObject(pObject, nbmodel, nbchild, nbsibling);

	return nbmodel;
}

int
wmain(int argc, wchar_t** argv)
{
	if (argc < 2)
	{
		puts("Error:\tDrag & drop a Ninja Chunk file (.sa2mdl) onto this .exe to convert!");
		Pause();
		return 1;
	}

	GetFileName(argv[1], FileName);

	/* Load given object file */

	NJS_CNK_OBJECT* object = LoadChunkObject(argv[1]);

	if (!object)
	{
		puts("Error:\tThe given Ninja Chunk file could not be loaded!");
		Pause();
		return 1;
	}

	/* Print .nja to stdout */

	printf("CNKOBJECT_START\n\n");

	PrintModObjectTree(object);

	printf(
		"CNKOBJECT_END\n\n"

		"DEFAULT_START\n\n"

		"#ifndef DEFAULT_OBJECT_NAME\n"
		"#define DEFAULT_OBJECT_NAME object_%s\n"
		"#endif\n\n"

		"DEFAULT_END\n\n\n", 

		FileName
	);

	/* Print complete, pause for copying */

	printf("Conversion of %i object(s) complete!\n", nbModels);

	Pause();

	return 0;
}