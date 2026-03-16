#include <Geode/Geode.hpp>
#include <Geode/modify/LevelCell.hpp>
#include <Geode/modify/LevelBrowserLayer.hpp>

using namespace geode::prelude;


static bool deezy_g_filterOn = false; // this one is a session var so it keeps the value when switching tabs but resets when game is closed


// unique key id for each level, im not using cvoltons api but i might switch later
static std::string getLevelKey(GJGameLevel* level) { 
    if (!level) return "";
    int id = level->m_levelID;
    if (id > 0) return "id_" + std::to_string(id);
    return "local_" + (level->m_levelName);
}



static bool isFav(GJGameLevel* level) {
    return Mod::get()->getSavedValue<bool>("fav_" + getLevelKey(level), false); // test

}
static void saveFav(GJGameLevel* level, bool isFav) {
    Mod::get()->setSavedValue("fav_" + getLevelKey(level), isFav); //this too
}





class $modify(MyLevelCell, LevelCell) {
    
    struct Fields {
        CCMenuItemSpriteExtra* heartBtn = nullptr;
    };
    
    
    void onHeart(CCObject* sender) {
        if (!m_level) return;
        bool fav = isFav(m_level);
        saveFav(m_level, !fav); // toggle the favorite status
    
            
            auto frame = CCSpriteFrameCache::get()->spriteFrameByName(!fav ? "gj_heartOn_001.png" : "gj_heartOff_001.png");
            auto spr = static_cast<CCSprite*>(m_fields->heartBtn->getNormalImage());
                spr->setDisplayFrame(frame); // pdate the sprite frame otherwise it can cause issues where the heart doesnt update
                //and gets stuck on one state (im doing the same thing in the else part in loadfromlevel)

        
    }


    
    
    
    void loadFromLevel (GJGameLevel* level) {
        LevelCell::loadFromLevel(level);
        if (level) {

                if (!level->m_isEditable) return; //this prevents crashes
                {
                                // test logs once again
            log::info("Loaded level: {}", level->m_levelName); 
            log::info("Level Name: {}, Is fav: {}", level->m_levelName, isFav(level));
                }

                
            

            if (!m_fields->heartBtn) {

                auto sprite = CCSprite::createWithSpriteFrameName(isFav(level) ? "gj_heartOn_001.png" : "gj_heartOff_001.png");            
                sprite->setScale(0.75f);        // set scale before button, "otherwise the hitbox can be too big (because it will be based on the default scale version)" - cvolton
                auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyLevelCell::onHeart)); 
                m_mainMenu->addChild(btn);    
                btn->setPosition(ccp(m_toggler->getPositionX() - 30.f, m_toggler->getPositionY() + 1.f));      // and offset it from the checkbox icon aka "select-toggler"
                m_fields->heartBtn = btn; // save the button in struct (fields)



            }

            else {
                
                auto frame = CCSpriteFrameCache::get()->spriteFrameByName(isFav(level) ? "gj_heartOn_001.png" : "gj_heartOff_001.png");
                auto spr = static_cast<CCSprite*>(m_fields->heartBtn->getNormalImage());
                    spr->setDisplayFrame(frame);
                m_fields->heartBtn->setScale(0.75f); // without this i see weird scaling issues (it scales up and doesnt go down) AGAIN

            }

        

       } 

    }
    
};


class $modify(MyLevelBrowserLayer, LevelBrowserLayer) {
    struct Fields {
        CCMenuItemSpriteExtra* filterBtn = nullptr; // the button


        bool filterActive = false; // filter active check
        bool isMyLevels = false; // my levels tab check
        int CurPage = 0;
        int TotPage = 1; 
        int pageSize = 10; 
        CCMenuItemSpriteExtra* FavPPage = nullptr;
        CCMenuItemSpriteExtra* FavNPage = nullptr;
        CCMenu* FavNPageO = nullptr;
        CCMenu* FavPPageO = nullptr;
    };
    


    void onFilter(CCObject* sender) {

        m_fields->filterActive = !m_fields->filterActive;
        deezy_g_filterOn = m_fields->filterActive;



        auto frame = CCSpriteFrameCache::get()->spriteFrameByName(m_fields->filterActive ? "gj_heartOn_001.png" : "gj_heartOff_001.png");
            auto spr = static_cast<CCSprite*>(m_fields->filterBtn->getNormalImage());
                spr->setDisplayFrame(frame); // pdate the sprite frame otherwise it can cause issues where the heart doesnt update
                //and gets stuck on one state (im doing the same thing in the else part in loadfromlevel)
                onRefresh(nullptr);


    }


    void onFavP(CCObject* ) 
    {
        m_fields->CurPage = m_fields->CurPage - 1;
     onRefresh(nullptr);

    }


    void onFavN(CCObject* ) 
    {

     m_fields->CurPage = m_fields->CurPage + 1;
     onRefresh(nullptr);

    
    }

    bool init(GJSearchObject* object) {

        m_fields->filterActive = deezy_g_filterOn; 
        m_fields->isMyLevels = (object->m_searchType == SearchType::MyLevels);


        


        if (!LevelBrowserLayer::init(object)) return false;
        if (object->m_searchType != SearchType::MyLevels) return true; // skip if not create tab

        // foward
        auto fpSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png"); 
        auto fpbtn = CCMenuItemSpriteExtra::create(fpSpr, this, menu_selector(MyLevelBrowserLayer::onFavP));
        auto fpmenu = CCMenu::create();
        fpmenu->addChild(fpbtn);
        auto ws = CCDirector::get()->getWinSize();
        fpmenu->setPosition(ccp(50.f, ws.height / 2.f));
        fpmenu->setVisible(false);
        this->addChild(fpmenu, 10);
        m_fields->FavPPageO = fpmenu;
        m_fields->FavPPage = fpbtn;

        // backward
        auto fnSpr = CCSprite::createWithSpriteFrameName("GJ_arrow_01_001.png");
        fnSpr->setFlipX(true);
        auto fnbtn = CCMenuItemSpriteExtra::create(fnSpr, this, menu_selector(MyLevelBrowserLayer::onFavN));
        auto fnmenu = CCMenu::create();
        fnmenu->addChild(fnbtn);
        fnmenu->setPosition(ccp(ws.width - 50.f, ws.height / 2.f));
        fnmenu->setVisible(false);
        this->addChild(fnmenu, 10);
        m_fields->FavNPageO = fnmenu;
        m_fields->FavNPage = fnbtn;

        

        
        // add filter button below


        auto sprite = CCSprite::createWithSpriteFrameName(m_fields->filterActive ? "gj_heartOn_001.png" : "gj_heartOff_001.png");
        auto btn = CCMenuItemSpriteExtra::create(sprite, this, menu_selector(MyLevelBrowserLayer::onFilter)); // just create the sprite as a button
        auto menu = CCMenu::create();
        menu->addChild(btn);
        auto myLevelsMenu = getChildByID("my-levels-menu");
            if (myLevelsMenu) {
            menu->setPosition(ccp(myLevelsMenu->getPositionX() + 39.f, myLevelsMenu->getPositionY() - 44.f));
        }
        this->addChild(menu, 10);   
        m_fields->filterBtn = btn;



        // im doing these 2 times  to get the variables in loadlevelsfinished AND init
        auto favs = collectFav();
        int total = (int)favs->count();
        int start = m_fields->CurPage * m_fields->pageSize;
        int end = std::min(start + m_fields->pageSize, total);
        std::string countStr = fmt::format("{} to {} of {}", start + 1, end, total);



        if (m_fields->filterActive) {
            if (auto n = getChildByID("next-page-menu")) n->setVisible(false);
            if (auto n = getChildByID("prev-page-menu")) n->setVisible(false);
            if (auto n = getChildByID("page-menu")) n->setVisible(false);
            if (m_fields->FavPPageO) m_fields->FavPPageO->setVisible(m_fields->CurPage > 0); // the if prevents crashes
            if (m_fields->FavNPageO) m_fields->FavNPageO->setVisible(m_fields->CurPage < m_fields->TotPage - 1); // same for this
            if (auto pageNum = getChildByID("level-count-label")) {
                    static_cast<CCLabelBMFont*>(pageNum)->setString(total == 0 ? "No favorites found" : countStr.c_str());
                }
            
            
        }

        
            return true;
    }





    CCArray* collectFav() //fav levels to ccarray
    {
        auto arr = CCArray::create(); 
        auto levels = LocalLevelManager::get()->m_localLevels; 
        for (unsigned int i = 0; i < levels->count(); i++) {      
            auto level = static_cast<GJGameLevel*>(levels->objectAtIndex(i)); 
            if (isFav(level)) {   
            arr->addObject(level); 
            }
        }
        return arr;                
    }

    

    void loadLevelsFinished(CCArray* levels, char const* key, int type) {
        

            if (m_fields->isMyLevels && m_fields->filterActive) { // if we are in my levels tab and filter is active, then we replace the levels with the collected fav levels
                 auto favs = collectFav();
                int total = (int)favs->count();
                m_fields->TotPage = (total + m_fields->pageSize - 1) / m_fields->pageSize; 

                m_fields->CurPage = std::max(0, std::min(m_fields->CurPage, m_fields->TotPage - 1)); // clamp 
                    int start = m_fields->CurPage * m_fields->pageSize;
                    int end = std::min(start + m_fields->pageSize, total);
                    auto pagedFavs = CCArray::create();
                    for (int i = start; i < end; i++) {
                        pagedFavs->addObject(favs->objectAtIndex(i));
                    }



                LevelBrowserLayer::loadLevelsFinished(pagedFavs, key, type);
                if (m_fields->FavPPageO) m_fields->FavPPageO->setVisible(m_fields->CurPage > 0); // the if prevents crashes
                if (m_fields->FavNPageO) m_fields->FavNPageO->setVisible(m_fields->CurPage < m_fields->TotPage - 1); // same for this
                // hide the vanilla arrow buttons
                if (auto n = getChildByID("next-page-menu")) n->setVisible(false);
                if (auto n = getChildByID("prev-page-menu")) n->setVisible(false);
                if (auto n = getChildByID("page-menu")) n->setVisible(false);

                // change the number at the corner 
                std::string countStr = fmt::format("{} to {} of {}", start + 1, end, total); //fmt format (thx john clang)
                if (auto pageNum = getChildByID("level-count-label")) {
                    static_cast<CCLabelBMFont*>(pageNum)->setString(countStr.c_str());
                }
                
                // if total is 0 say no favorites found instead
                if (auto pageNum = getChildByID("level-count-label")) {
                    static_cast<CCLabelBMFont*>(pageNum)->setString(total == 0 ? "No favorites found" : countStr.c_str());
                }
                


                



                if (auto list = getChildByID("GJListLayer"))
                    if (auto title = static_cast<CCLabelBMFont*>(list->getChildByID("title")))
                        title->setString("My Favorited Levels");

                
            

                return;




            }

            else 
            {

                if (m_fields->FavPPageO) m_fields->FavPPageO->setVisible(false); // hide custom buttons if not in fav mode
                if (m_fields->FavNPageO) m_fields->FavNPageO->setVisible(false);
                // show the vanilla arrow buttons
                if (auto n = getChildByID("next-page-menu")) n->setVisible(true);
                if (auto n = getChildByID("prev-page-menu")) n->setVisible(true);
                if (auto n = getChildByID("page-menu")) n->setVisible(true);


                



                
            }
            LevelBrowserLayer::loadLevelsFinished(levels, key, type); //call normally if not in tab / filter is on
    }

};