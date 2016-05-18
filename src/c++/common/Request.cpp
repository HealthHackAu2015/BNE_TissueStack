/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "tissuestack.h"

tissuestack::common::Request::Request() : _type(tissuestack::common::Request::Type::RAW_HTTP) {};

tissuestack::common::Request::~Request()
{
	// nothing to be done, after all we are abstract
};

const tissuestack::common::Request::Type tissuestack::common::Request::getType() const
{
	return this->_type;
}

void tissuestack::common::Request::setType(tissuestack::common::Request::Type type)
{
	this->_type = type;
}