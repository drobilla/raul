/*
  Copyright 2007-2013 David Robillard <d@drobilla.net>

  Raul is free software: you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation, either version 3 of the License, or any later version.

  Raul is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Raul.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RAUL_DELETABLE_HPP
#define RAUL_DELETABLE_HPP

namespace Raul {

/** Something with a virtual destructor.
 *
 * \ingroup raul
 */
class Deletable
{
public:
  Deletable() = default;

  Deletable(const Deletable&) = default;
  Deletable& operator=(const Deletable&) = default;

  Deletable(Deletable&&) = default;
  Deletable& operator=(Deletable&&) = default;

  virtual ~Deletable() = default;
};

} // namespace Raul

#endif // RAUL_DELETABLE_HPP
