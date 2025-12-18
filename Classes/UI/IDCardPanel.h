#ifndef __ID_CARD_PANEL_H__
#define __ID_CARD_PANEL_H__

#include "cocos2d.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocos2d::ui;

class IDCardPanel : public Node {
public:
    static Node* createPanel(Node* backgroundLayer);  

private:
    static Node* createResourceLabel(
        const std::string& typeName,
        const std::string& iconPath,
        float minheight
    );
};

#endif // __ID_CARD_PANEL_H__
