#include "IDCardPanel.h"
#include "Core/Core.h"

Node* IDCardPanel::createResourceLabel(
    const std::string& text,
    const std::string& iconPath,
    float minHeight,
    const std::string& valueName
)
{
    float paddingY = minHeight * 0.15f;
    float paddingX = minHeight * 0.35f;
    float spacing  = 8.0f;

    auto label = Label::createWithTTF(
        text,
        "fonts/ScienceGothic.ttf",
        minHeight * 0.85f
    );
    label->setAnchorPoint(Vec2(0, 0.5f));
    if (!valueName.empty())
    {
        label->setName(valueName);
    }

    Size textSize = label->getContentSize();

    Sprite* icon = nullptr;
    float iconWidth = 0.0f;

    if (!iconPath.empty())
    {
        icon = Sprite::create(iconPath);
        if (icon)
        {
            float iconTargetHeight = minHeight * 0.8f;
            float scale = iconTargetHeight / icon->getContentSize().height;
            icon->setScale(scale);
            iconWidth = icon->getContentSize().width * scale;
            icon->setAnchorPoint(Vec2(0, 0.5f));
            icon->setName("iconSprite");
        }
    }
    float height = minHeight;

    float width =
        paddingX * 2 +
        iconWidth +
        (icon ? spacing : 0) +
        textSize.width;

    // 固定在左上角区域，避免遮挡主场景
    auto panel = Node::create();
    panel->setContentSize(Size(width, height));
    panel->setIgnoreAnchorPointForPosition(false);
    panel->setAnchorPoint(Vec2(0, 1));   

    auto bg = DrawNode::create();
    bg->drawSolidRect(
        Vec2(0, -height),
        Vec2(width, 0),
        Color4F(0.15f, 0.15f, 0.2f, 0.75f)
    );
    panel->addChild(bg);

    auto innerBg = DrawNode::create();
    innerBg->drawSolidRect(
        Vec2(2, -height + 2),
        Vec2(width - 2, -2),
        Color4F(0.2f, 0.4f, 0.9f, 0.4f)
    );
    panel->addChild(innerBg);

    float cursorX = paddingX;

    if (icon)
    {
        icon->setPosition(Vec2(cursorX, -height / 2));
        panel->addChild(icon);
        cursorX += iconWidth + spacing;
    }

    label->setPosition(Vec2(cursorX, -height / 2));
    panel->addChild(label);

    return panel;
}

Node* IDCardPanel::createPanel(Node* backgroundLayer)
{
    auto visibleSize = Director::getInstance()->getVisibleSize();
    auto origin = Director::getInstance()->getVisibleOrigin();

    float panelWidth = visibleSize.width * 0.32f;
    float panelHeight = visibleSize.height * 0.18f;
    float margin = 24.0f;

  
    auto panel = Node::create();
    panel->setPosition(Vec2(
        origin.x + margin + panelWidth / 2,
        origin.y + visibleSize.height - margin - panelHeight / 2
    ));
    backgroundLayer->addChild(panel, 1);

    auto bg = Node::create();
    panel->addChild(bg, 0);

    auto bgRect = Sprite::create("UI/ID_card_background.png");
    if (bgRect)
    {
        bgRect->setAnchorPoint(Vec2(0.5f, 0.5f));
        bg->setPosition(Vec2(10, 0)); 

        Size bgSize = bgRect->getContentSize();

        bgRect->setScaleX(panelWidth / bgSize.width);
        bgRect->setScaleY(panelHeight / bgSize.height);

        bg->addChild(bgRect);
    }


    auto title = Label::createWithTTF(
        "ID Card",
        "fonts/ScienceGothic.ttf",
        22
    );
    title->setPosition(Vec2(0, panelHeight / 2 - 10));
    bg->addChild(title, 2);

    float contentTopY = panelHeight / 2 -5;

    auto photoFrame = Sprite::create("UI/ID_card_photo_frame.png");
    float photoWidth = 0.0f;

    if (photoFrame)
    {
        float targetHeight = panelHeight * 0.25f;
        float scale = targetHeight / photoFrame->getContentSize().height;
        photoFrame->setScale(scale);

        photoWidth = photoFrame->getContentSize().width * scale;

        photoFrame->setAnchorPoint(Vec2(0, 1)); 
        photoFrame->setPosition(Vec2(
            -panelWidth / 2 + 5,
            contentTopY -15
        ));
        bg->addChild(photoFrame, 2);
    }

    float infoX = -panelWidth / 2  + photoWidth +10;

    int gold = Core::getInstance()->getResource(ResourceType::COIN);
    int diamond = Core::getInstance()->getResource(ResourceType::DIAMOND);

    char goldText[64];
    snprintf(goldText, sizeof(goldText), "Gold: %d", gold);
    auto goldLabel = createResourceLabel(
        goldText,
        "source/coin/coin_0001.png", 
        15,
        "valueLabel"
    );
    goldLabel->setName("goldPanel");
    goldLabel->setAnchorPoint(Vec2(0, 1));
    goldLabel->setPosition(Vec2(
        infoX,
        contentTopY
    ));
    bg->addChild(goldLabel, 2);

    auto goldIcon = dynamic_cast<Sprite*>(goldLabel->getChildByName("iconSprite"));
    if (goldIcon) {
        Core::getInstance()->playResourceAnimation(goldIcon, ResourceType::COIN, 0.12f, true);
    }

    char diamondText[64];
    snprintf(diamondText, sizeof(diamondText), "Diamonds: %d", diamond);
    auto diamondLabel = createResourceLabel(
        diamondText,
        "source/diamond/sprite_0000.png",
        15,
        "valueLabel"
    );
    diamondLabel->setName("diamondPanel");
    diamondLabel->setAnchorPoint(Vec2(0, 1));
    diamondLabel->setPosition(Vec2(
        infoX,
        contentTopY - 15
    ));
    bg->addChild(diamondLabel, 2);

    auto diamondIcon = dynamic_cast<Sprite*>(diamondLabel->getChildByName("iconSprite"));
    if (diamondIcon) {
        Core::getInstance()->playResourceAnimation(diamondIcon, ResourceType::DIAMOND, 0.15f, true);
    }

    return panel;
}
