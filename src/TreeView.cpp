#include <algorithm>

#include "TreeView.h"
#include "video.h"
#include "textUI.h"
#include "directory.h"

#include "noggit.h" // arial12

TreeViewButton::TreeViewButton( float _x, float _y, TreeView* pTreeView ) : buttonUI(	_x, _y, 12.0f, 12.0f, "Interface\\Buttons\\UI-PlusButton-Up.blp", "Interface\\Buttons\\UI-PlusButton-Down.blp" ), mTreeView( pTreeView )
{
}

frame * TreeViewButton::processLeftClick( float /*mx*/, float /*my*/ )
{
	SetClicked( true );
	mTreeView->Toggle();
	return this;
}

void TreeViewButton::SetClicked( bool pClicked )
{
	clicked = pClicked;
}

TreeView::TreeView( float pX, float pY, Directory * pDirectory, TreeView * pParent, void (*pSelectFunction)( const std::string& ) ) : frame( pX, pY, 0.0f, 0.0f ), mParent( pParent ), mMyDir( pDirectory ), mSelectFunction( pSelectFunction ), mExpanded( false )
{
	mMyButton = new TreeViewButton( 0, 0, this );
	mMyText = new textUI( 13, 0, mMyDir->mName, &arial12, eJustifyLeft );

	float lY = 13.0f;

	std::map<std::string,Directory*>::iterator it;
	for( it = mMyDir->mSubdirectories.begin(); it != mMyDir->mSubdirectories.end(); ++it )
	{
		mOthers.push_back( new TreeView( 13.0f, lY, it->second, this, pSelectFunction ) );
		lY = lY + 13.0f;
	}
	std::vector<File*>::iterator itfile;
	for( itfile = mMyDir->mSubfiles.begin(); itfile != mMyDir->mSubfiles.end(); ++itfile )
	{
		textUI * temp = new textUI( 13.0f, lY, (*itfile)->mName, &arial12, eJustifyLeft );
		mFiles.push_back( temp );
		lY = lY + 13.0f;
	}
}

void TreeView::Expand()
{
	mExpanded = true;
}

void TreeView::Minimize()
{
	mExpanded = false;
}

bool TreeView::Expanded()
{
	return mExpanded;
}

std::string TreeView::GetDirectoryName()
{
	return mMyDir->mName;
}

void TreeView::Move( int pEntries, TreeView * pFrom )
{
	std::vector<TreeView*>::iterator trees = std::find( mOthers.begin(), mOthers.end(), pFrom );
	trees++;
	while( trees != mOthers.end() )
	{
		(*trees)->y = (*trees)->y + pEntries * 13;
		trees++;
	}

	std::vector<textUI*>::iterator childfiles;
	for( childfiles = mFiles.begin(); childfiles != mFiles.end(); ++childfiles )
	{
		(*childfiles)->y = (*childfiles)->y + pEntries * 13;
	}

	if( mParent )
	{
		mParent->Move( pEntries, this );
	}
}

void TreeView::Toggle()
{
	mExpanded = !mExpanded;
	mMyButton->SetClicked( mExpanded );

	std::vector<TreeView*>::iterator childtreeviews;
	for( childtreeviews = mOthers.begin(); childtreeviews != mOthers.end(); ++childtreeviews )
	{
		if( (*childtreeviews)->Expanded() )
			(*childtreeviews)->Toggle();
	}
	
	if( mParent )
		if( !mExpanded )
			mParent->Move( -int( ( mOthers.size() + mFiles.size() ) ), this );
		else
			mParent->Move( ( mOthers.size() + mFiles.size() ), this );
}

void TreeView::render() const
{
	if( hidden )
		return;

	glPushMatrix();
	glTranslatef( x, y, 0 );
	mMyButton->render();
	mMyText->render();

	if( mExpanded )
	{
		std::vector<TreeView*>::const_iterator childtreeviews;
		for( childtreeviews = mOthers.begin(); childtreeviews != mOthers.end(); ++childtreeviews )
		{
			(*childtreeviews)->render();
		}
		std::vector<textUI*>::const_iterator childfiles;
		for( childfiles = mFiles.begin(); childfiles != mFiles.end(); ++childfiles )
		{
			(*childfiles)->render();
		}
	}

	glPopMatrix();
}

void TreeView::SetSelectFunction( void (*pSelectFunction)( const std::string& ) )
{
	mSelectFunction = pSelectFunction;
}

frame * TreeView::processLeftClick( float mx, float my )
{
	if( hidden )
		return 0;

	mx -= x;
	my -= y;
	if((!mMyButton->hidden)&&(mMyButton->x<mx)&&(mMyButton->x+mMyButton->width>mx)&&(mMyButton->y<my)&&(mMyButton->y+mMyButton->height>my))
		return mMyButton->processLeftClick(mx-mMyButton->x,my-mMyButton->y);

	std::vector<textUI*>::iterator childfiles;
	if( mSelectFunction )
		for( childfiles = mFiles.begin(); childfiles != mFiles.end(); ++childfiles )
		{
			if( (!(*childfiles)->hidden)&&((*childfiles)->x<mx)&&((*childfiles)->x+(*childfiles)->width>mx)&&((*childfiles)->y<my)&&((*childfiles)->y+(*childfiles)->height>my) )
			{
				std::string lPath;
				TreeView * lParent = this;
				while( lParent )
				{
					lPath.insert( 0, std::string( lParent->GetDirectoryName() + "/" ) );
					lParent = lParent->GetParent();
				}
				mSelectFunction( lPath + (*childfiles)->getText() );
				return *childfiles;
			}
		}

	if( mExpanded )
	{
		std::vector<TreeView*>::iterator childtreeviews;
		for( childtreeviews = mOthers.begin(); childtreeviews != mOthers.end(); ++childtreeviews )
		{
			(*childtreeviews)->processLeftClick(mx,my);
		}
	}

	return 0;
}
