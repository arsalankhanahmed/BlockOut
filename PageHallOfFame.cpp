/*
   File:        PageHallOfFame.cpp
  Description: Hall of Fame page
  Program:     BlockOut
  Author:      Jean-Luc PONS

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
*/

#include "Menu.h"
#include <fstream>
#include <vector>
#include <curl.h>

#include <iostream>
#include <string>
#include <system.h>
#include <system.h>
using namespace std;

void PageHallOfFame::Prepare(int iParam,void *pParam) {

  nbItem  = 10;
  if(iParam<10)
    selItem = iParam;
  else
    selItem = 0;

  strcpy(editText,"          ");
  editPos = 0;
  startEditTime = 0.0f;
  editCursor = FALSE;
  editScore = (SCOREREC *)pParam;
  mParent->GetSetup()->GetHighScore(allScore);

  if( editScore ) {
    editMode = TRUE;
  } else {
    editMode = FALSE;
  }

}

PLAYER_INFO *PageHallOfFame::GetPlayer() {

  strcpy(pInfo.name , allScore[selItem].name);
  int i = (int)strlen(pInfo.name) - 1;
  while(i>=0 && pInfo.name[i]==' ') {
    pInfo.name[i]=0;
    i--;
  }
  pInfo.highScore = allScore[0].score;
  pInfo.rank = selItem + 1;

  return &pInfo;

}

void PageHallOfFame::Render() {

  char tmp[256];

  sprintf(tmp,"HALL OF FAME %s",mParent->GetSetup()->GetName());
  mParent->RenderTitle(tmp);
  for(int i=0;i<10;i++) {
    sprintf(tmp,"%2d ",i+1);
    mParent->RenderText(0,i,(selItem==i) && !editMode,tmp);
    mParent->RenderText(3,i,(selItem==i) && !editMode,allScore[i].name);
    sprintf(tmp,"%7d ",allScore[i].score);
    mParent->RenderText(13,i,(selItem==i) && !editMode,tmp);
    sprintf(tmp,"[%s]",FormatDateShort(allScore[i].date));
    mParent->RenderText(21,i,(selItem==i) && !editMode,tmp);
  }
  // Edition mode
  if( editMode ) {
    mParent->RenderText(3,selItem,FALSE,editText);
    mParent->RenderText(3+editPos,selItem,editCursor,STR(" "));
  }

}

int PageHallOfFame::Process(BYTE *keys,float fTime) {

  if( !editMode ) {

    ProcessDefault(keys,fTime);

    if( keys[SDLK_RETURN] ) {
      mParent->ToPage(&mParent->scoreDetailsPage,selItem,allScore + selItem);
      keys[SDLK_RETURN] = 0;
    }

    if( keys[SDLK_ESCAPE] ) {
       mParent->ToPage(&mParent->mainMenuPage);
       keys[SDLK_ESCAPE] = 0;
    }

  } else {
    ProcessEdit(keys,fTime);
  }

  return 0;

}

// ---------------------------------------------------------------------

void PageHallOfFame::ProcessEdit(BYTE *keys,float fTime) {
  
  if( startEditTime == 0.0f )
    startEditTime = fTime;

  editCursor = ( (fround((startEditTime - fTime) * 2.0f)%2) == 0 );

  char c = GetChar(keys);
  if( c>0 && editPos<10 ) {
    editText[editPos] = c;
    editPos++;
  }
  
  // Delete
  if( keys[SDLK_DELETE] || keys[SDLK_LEFT] || keys[SDLK_BACKSPACE] ) {
    if( editPos>0 ) editPos--;
    if( editPos<10 ) editText[editPos]=' ';
    keys[SDLK_DELETE] = 0;
    keys[SDLK_LEFT] = 0;
    keys[SDLK_BACKSPACE] = 0;
  }

  if( keys[SDLK_ESCAPE] || keys[SDLK_RETURN] ) {
    // Record new name and save
    strcpy(editScore->name,editText);
    mParent->GetSetup()->SaveHighScore();
    mParent->GetSetup()->GetHighScore(allScore);
    
    string myText;
    ifstream urlFile(LID((char*)"url.txt"));
    string url="";
    while (getline(urlFile, myText)) {
         url += myText;
    }
   urlFile.close();
   std::string data = "";

    data += "setup=";
    data += SetupManager().GetName();

    data += "&date=";
    data += FormatDateShort(editScore->date);

    data += "&gametime=";
    data += std::to_string(editScore->gameTime);

    data += "&name=";
    data += editScore->name;

    data+= "&cube=";
    data+= std::to_string(editScore->nbCube);

    data+= "&line1=";
    data+= std::to_string(editScore->nbLine1);

    data+= "&line2=";
    data+= std::to_string(editScore->nbLine2);

    data+= "&line3=";
    data+= std::to_string(editScore->nbLine3);

    data+= "&line4=";
    data+= std::to_string(editScore->nbLine4);

    data+= "&line5=";
    data+= std::to_string(editScore->nbLine5);

    data+= "&next=";
    data += "0";//editScore->next.;

    data+= "&score=";
    data+= std::to_string(editScore->score);

    data+= "&startlevel=";
    data+= std::to_string(editScore->startLevel);

    ofstream response_flie;
    response_flie.open("response_file.txt");
    response_flie << url;
    CURL* curl;
    CURLcode res;

    /* In windows, this will init the winsock stuff */
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if (curl) {
        /* First set the URL that is about to receive our POST. This URL can
           just as well be a https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL, url);
        /* Now specify the POST data */
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());//"name=daniel&project=curl");
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, false);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);

        /* Perform the request, res will get the return code */
        res = curl_easy_perform(curl);
        //response_flie << res;
        /* Check for errors */
        if (res != CURLE_OK)
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
    response_flie.close();
    editMode = FALSE;
    keys[SDLK_ESCAPE] = 0;
    keys[SDLK_RETURN] = 0;
  }

}

