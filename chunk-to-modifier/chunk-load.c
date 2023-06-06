#include <chunk-to-modifier.h>

#include <stdlib.h>

#define CURR_FILE_VERSION 3
#define FORMAT_MASK 0xFFFFFFFFFFFFFFu
#define CNK_OBJ 0x4C444D324153u
#define HEADER_SIZE 16

#define SetPointer(PTR, OFFSET) (*(sint32*)&PTR = *(sint32*)&PTR + OFFSET)

#define ChkPointer(PTR, OFFSET) (PTR != NULL && (sint32)PTR < OFFSET)

static void
SetChunkObjectPointers(NJS_CNK_OBJECT* pobj, sint32 offset)
{
	if (ChkPointer(pobj->model, offset))
	{
		SetPointer(pobj->model, offset);

		NJS_CNK_MODEL* model = pobj->model;

		SetPointer(model->vlist, offset);
		SetPointer(model->plist, offset);
	}
	if (ChkPointer(pobj->child, offset))
	{
		SetPointer(pobj->child, offset);
		SetChunkObjectPointers(pobj->child, offset);
	}
	if (ChkPointer(pobj->sibling, offset))
	{
		SetPointer(pobj->sibling, offset);
		SetChunkObjectPointers(pobj->sibling, offset);
	}
}

NJS_CNK_OBJECT*
LoadChunkObject(const wchar_t* fn)
{
	FILE* f = _wfopen(fn, L"rb");

	if (!f)
		return NULL;

	uint64 magic;

	fread(&magic, sizeof(magic), 1, f);

	uint8 version = magic >> 56;

	magic &= FORMAT_MASK;

	if (version != CURR_FILE_VERSION || magic != CNK_OBJ) {
		fclose(f);
		return NULL;
	}

	uint32 modeloff;

	fread(&modeloff, sizeof(modeloff), 1, f);
	modeloff -= HEADER_SIZE;

	uint32 tempaddr;

	fread(&tempaddr, sizeof(tempaddr), 1, f);

	sint32 mdlsize = tempaddr - HEADER_SIZE;

	void* buf = malloc(mdlsize);

	fread(buf, 0x01, mdlsize, f);

	NJS_CNK_OBJECT* pobj = (NJS_CNK_OBJECT*)((sint32)buf + modeloff);

	sint32 offset = (sint32)buf - HEADER_SIZE;

	SetChunkObjectPointers(pobj, offset);

	fclose(f);

	return pobj;
}