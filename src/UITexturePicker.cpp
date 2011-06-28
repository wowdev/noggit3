#include "UITexturePicker.h"

#include <iostream>
#include <sstream>

#include "Log.h"
#include "MapChunk.h"
#include "Noggit.h" // arial14, arialn13
#include "Selection.h"
#include "UIMapViewGUI.h"
#include "UIMinimizeButton.h"
#include "UIText.h" // UIText
#include "UITexture.h"
#include "UITexturingGUI.h"
#include "UIToolbar.h"
#include "UIWindow.h"

void textureClick( UIFrame* f,int id )
{
  // redirect to sender object.
  (reinterpret_cast<UITexturePicker *>(f->parent))->setTexture(id);
}

UITexturePicker::UITexturePicker( int xPos, int yPos, int w, int h, UIMapViewGUI *setGui )
: UIWindow( xPos, yPos, w, h )
{
  this->mainGUI = setGui;

  this->tex1 = new UITexture(10,30,110,110,"tileset\\generic\\black.blp");
  this->tex2 = new UITexture(130,30,110,110,"tileset\\generic\\black.blp");
  this->tex3 = new UITexture(250,30,110,110,"tileset\\generic\\black.blp");
  this->tex4 = new UITexture(370,30,110,110,"tileset\\generic\\black.blp");

  this->tex1->setClickFunc(textureClick,0);
  this->tex2->setClickFunc(textureClick,1);
  this->tex3->setClickFunc(textureClick,2);
  this->tex4->setClickFunc(textureClick,3);

  this->addChild(this->tex1);
  this->addChild(this->tex2);
  this->addChild(this->tex3);
  this->addChild(this->tex4);
  this->addChild(new UIText( 10.0f, 9.0f, "Pick one of the textures.", arial14, eJustifyLeft ) );

  //close button
  this->addChild( new UIMinimizeButton( w, this ) );

}

void UITexturePicker::getTextures(nameEntry *lSelection)
{
  this->hidden = false;
  if( lSelection )
    if( lSelection->type == eEntry_MapChunk )
    {
        if(lSelection->data.mapchunk->nTextures > 0)
        {
          this->tex1->setTexture(lSelection->data.mapchunk->textures[0]);
          this->tex1->hidden = false;
        }
        else this->tex1->hidden = true;
        if(lSelection->data.mapchunk->nTextures > 1)        
        {
          this->tex2->setTexture(lSelection->data.mapchunk->textures[1]);
          this->tex2->hidden = false;
        }
        else this->tex2->hidden = true;
        if(lSelection->data.mapchunk->nTextures > 2)        
        {
          this->tex3->setTexture(lSelection->data.mapchunk->textures[2]);
          this->tex3->hidden = false;
        }
        else this->tex3->hidden = true;
        if(lSelection->data.mapchunk->nTextures > 3)
        {
          this->tex4->setTexture(lSelection->data.mapchunk->textures[3]);
          this->tex4->hidden = false;
        }
        else this->tex4->hidden = true;
    }
}

void UITexturePicker::setTexture(int id)
{
  OpenGL::Texture* curTex = NULL;

  switch (id)
  {
  case 0:
    curTex = this->tex1->getTexture();  
  break;
  case 1:
    curTex = this->tex2->getTexture();  
  break;
  case 2:
    curTex = this->tex3->getTexture();  
  break;
  case 3:
    curTex = this->tex4->getTexture();  
  break;
  }

  if(curTex)
  {
    UITexturingGUI::setSelectedTexture(curTex);  
    if( UITexturingGUI::getSelectedTexture() )
    {
      UITexturingGUI::updateSelectedTexture();
    }
  }
}
