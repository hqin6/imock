#include "DatNodeFactory.h"
#include "DatNode.h"

Node* DatNodeFactory::Get(TiXmlElement* e)
{
    return new DatNode();
}
