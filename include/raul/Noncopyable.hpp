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

#ifndef RAUL_NONCOPYABLE_HPP
#define RAUL_NONCOPYABLE_HPP

namespace Raul {

class Noncopyable
{
public:
  Noncopyable(const Noncopyable&) = delete;
  const Noncopyable& operator=(const Noncopyable&) = delete;

  Noncopyable(Noncopyable&&) = delete;
  Noncopyable& operator=(Noncopyable&&) = delete;

protected:
  Noncopyable()  = default;
  ~Noncopyable() = default;
};

} // namespace Raul

#endif // RAUL_NONCOPYABLE_HPP
