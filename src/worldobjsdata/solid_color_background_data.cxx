//  $Id: solid_color_background_data.cxx,v 1.4 2003/02/18 01:23:52 grumbel Exp $
//
//  Pingus - A free Lemmings clone
//  Copyright (C) 2000 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <iostream>
#include "../xml_file_reader.hxx"
#include "../xml_file_writer.hxx"
#include "../editorobjs/solid_color_background_obj.hxx"
#include "../worldobjs/solid_color_background.hxx"
#include "solid_color_background_data.hxx"

namespace WorldObjsData {

SolidColorBackgroundData::SolidColorBackgroundData (xmlDocPtr doc, xmlNodePtr cur)
{
  XMLFileReader reader(doc, cur);
  reader.read_color("color", color);
}

SolidColorBackgroundData::SolidColorBackgroundData (const SolidColorBackgroundData& old)
  : WorldObjData(old),
    pos(old.pos),
    color(old.color)
{
}

void
SolidColorBackgroundData::write_xml (std::ostream& xml)
{
  XMLFileWriter writer(xml);
  writer.begin_section("worldobj", "type=\"solidcolor-background\"");
  writer.write_color("color", color);
  writer.end_section();
}

WorldObj* 
SolidColorBackgroundData::create_WorldObj ()
{
  return new WorldObjs::SolidColorBackground(*this);
}

EditorObjLst 
SolidColorBackgroundData::create_EditorObj()
{
  return EditorObjLst(1, new EditorObjs::SolidColorBackgroundObj(*this));
}

} // namespace WorldObjsData

/* EOF */
