/***************************************************************************                                              
 *   Copyright (C) 2011 Joven Club de Computacion y Electronica V.C        *                                      
 *   Written by Darien Alonso Camacho <darienad030111@vcl.jovenclub.cu>    *                                         
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.           *
 **************************************************************************/

#include <mingpp.h>
#include "TinyXml/tinyxml.h"
#include "timeline/timeline.h"
#include <iostream>
#include <cstring>
#include <vector>
using namespace std;

int width = 0;          //The width for the swf movie
int height = 0;         //The height for the swf movie
int r = 0;              //Red component of RGB color
int g = 0;              //Green component of RGB color
int b = 0;              //Blue component of RGB color
int framerate = 0;      //The framerate
SWFMovie* swfmovie;     //The swf movie to output
Timeline* timeline;     //The timeline of the swf movie


//Dump all the nodes of the xml file
int dumpXML( TiXmlNode* pParent );

//Parse the XML files and dump content to SWF format
int dumpToSwf( TiXmlNode* pParent, char* movie_name  );

//----------------------------------------------------------------------------------------------
int main(int argc, char *argv[])
{

    if( argc != 3 )
    {
        cout<< "Error in arguments." << endl;
        return 1;
    }

    TiXmlDocument doc( argv[ 1 ] );
    bool load_ok = doc.LoadFile();

    if( load_ok )
    {
        dumpToSwf( &doc, argv[ 2 ] );
    }
    else
    {
        cout<< "Failed to load the file" << endl;
    }

    return 0;
}

//-----------------------------------------------------------------------




//--------------------------------------------------------------------------------------------
int dumpXML( TiXmlNode* pParent )
{
    if( !pParent )
        return -1;

    TiXmlElement* document = 0;
    TiXmlElement* first_child = 0;      //Can be <symbols> or <timeline>
    TiXmlElement* timeline_tag = 0;
    TiXmlElement* layers_tag = 0;
    TiXmlElement* one_layer_tag = 0;
    TiXmlElement* frames_tag = 0;
    TiXmlElement* one_frame_tag = 0;
    TiXmlElement* elements_tag = 0;
    TiXmlElement* one_element_tag = 0;
    TiXmlAttribute* pAttrib = 0;
    TiXmlAttribute* attrib = 0;
    int duration = 0;
    int layer_index = 0;
    int frame_index = 0;
    bool empty_frame = false;

    document = pParent->FirstChildElement();	//Go into document
    if( !document )
        return -1;

    if( document->ValueStr() == "document" )
    {
        pAttrib = document->FirstAttribute();
        while( pAttrib )
        {
         if( strcmp( pAttrib->Name(), "width" ) == 0  )
         {
            width = pAttrib->IntValue();
         }
         else if( strcmp( pAttrib->Name(), "height" ) == 0 )
         {
            height = pAttrib->IntValue();
         }
         else if( strcmp( pAttrib->Name(), "backgroundColor" ) == 0 )
         {
            const char* rgb_color_str = pAttrib->Value();
            Frame::rgbStrToRgbDec( rgb_color_str, r, g, b );
         }
         else if( strcmp( pAttrib->Name(), "framerate" ) == 0 )
         {
            framerate = pAttrib->IntValue();
         }

         pAttrib = pAttrib->Next();
        }//End while

        first_child = document->FirstChildElement();
        if( !first_child )
            return -1;

        //Check if "symbols" tag exists
        if( first_child->ValueStr() == "symbols" )
        {
            timeline_tag = first_child->NextSiblingElement();
            //Include symbols
        }
        else
        {
            timeline_tag = first_child;
            cout<< "cz2swf warning: There is not <symbols> tag, any <symbolInstance> tag will be ignored." << endl;
        }

        //Check if "timeline" tag exists
        if( timeline_tag->ValueStr() == "timeline" )
        {
            layers_tag = timeline_tag->FirstChildElement();
            if( !layers_tag )
                return -1;

            if( layers_tag->ValueStr() == "layers" )
            {
                one_layer_tag = layers_tag->FirstChildElement();    //First layer
                if( !one_layer_tag )
                    return -1;

                if( one_layer_tag->ValueStr() == "layer" )
                {
                  while( one_layer_tag )
                  {
                      attrib = one_layer_tag->FirstAttribute();
                      if( !attrib )
                          return -1;

                      while( attrib )
                      {
                            if( strcmp( attrib->Name(), "index" ) == 0 )
                            {
                                layer_index = attrib->IntValue();
                            }
                            /*if( strcmp( attrib->Name(), "name" ) == 0 )
                            {
                                        //Do something
                            }  */
                            attrib = attrib->Next();
                      }

                    //Append a new layer
                    timeline->addLayer();

                    frames_tag = one_layer_tag->FirstChildElement();    //<frames> tag
                    if( !frames_tag )
                        return -1;

                    if( frames_tag->ValueStr() == "frames" )
                    {
                        one_frame_tag = frames_tag->FirstChildElement();
                        if( !one_frame_tag )
                            return -1;

                        if( one_frame_tag->ValueStr() == "frame" )  //First frame
                        {
                            while( one_frame_tag )
                            {
                                attrib = one_frame_tag->FirstAttribute();
                                if( !attrib )
                                    return -1;

                                while( attrib )
                                {
                                    if( strcmp( attrib->Name(), "index" ) == 0 )
                                    {cout<< "frame_index= " << endl;
                                        frame_index = attrib->IntValue();
                                    }
                                    else if( strcmp( attrib->Name(), "empty" ) == 0 )
                                    {
                                        if( attrib->IntValue() == 1 )
                                            empty_frame = true;
                                        else
                                            empty_frame = false;
                                    }
                                    else if( strcmp( attrib->Name(), "duration" ) == 0 )
                                    {
                                        duration = attrib->IntValue(); 
                                    }
                                    attrib = attrib->Next();
                                }
                                /*
                                   Frames settings has to be set here before the <elements> tag
                                 */

                                timeline->completeFrames( layer_index, frame_index, duration );
                                int remove_pos = 0;
                                duration ++;
                                remove_pos = frame_index + duration;
                                timeline->setFrameDuration( layer_index, frame_index, remove_pos );

                            if( !empty_frame )
                            {
                                elements_tag = one_frame_tag->FirstChildElement();
                                if( !elements_tag )
                                    return -1;

                                if( elements_tag->ValueStr() == "elements" )
                                {
                                    one_element_tag = elements_tag->FirstChildElement();
                                    if( !one_element_tag )
                                        return -1;

                                    while( one_element_tag )
                                    {
                                        timeline->addElement( layer_index, frame_index, one_element_tag );
                                        //Get the following element
                                        one_element_tag = one_element_tag->NextSiblingElement();                                        
                                    }
                                }
                                else
                                {
                                    cout<< "cz2swf: Bad document structure, no <elements> tag." << endl;
                                    return -1;
                                }
                            }

                                //Get next frame
                                one_frame_tag = one_frame_tag->NextSiblingElement();
                            }
                        }
                    }
                    else
                    {
                        cout<< "cz2swf: Bad document structure, no <frames> tag." << endl;
                        return -1;
                    }
                    //remove items from layer layer_index here

                    //Get next layer
                    one_layer_tag = one_layer_tag->NextSiblingElement();    //The others layers
                  }
                }
            }
            else
            {
                cout<< "cz2swf: Bad document structure, no <layers> tag." << endl;
                return -1;
            }
        }
        else
        {
            cout<< "cz2swf: Bad document structure, no <timeline> tag." << endl;
            return -1;
        }
    }
    else
    {
        cout<< "cz2swf: Bad document structure, no <document> tag." << endl;
        return -1;
    }

}
//-------------------------------------------------------------------------
/*
  Dump content to SWF format
*/
int dumpToSwf( TiXmlNode* pParent, char* movie_name  )
{
    swfmovie = new SWFMovie( 8 );
    timeline = new Timeline( swfmovie );

    int result = dumpXML( pParent );    

    timeline->dumpMovie();

    swfmovie->setDimension( width, height );
    swfmovie->setRate( framerate );

    swfmovie->setBackground( r, g, b );

    //9 = maximum compression
    swfmovie->save( movie_name,  9 );

    delete swfmovie;
}









