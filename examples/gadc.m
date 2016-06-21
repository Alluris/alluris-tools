## Copyright (C) 2015 Alluris GmbH & Co. KG <weber@alluris.de>
##
## This file is part of liballuris.
##
## Liballuris is free software: you can redistribute it and/or modify
## it under the terms of the GNU Lesser General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## Liballuris is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with liballuris. See ../COPYING.LESSER
## If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*-
## @deftypefn  {Function File} {@var{r} =} gadc (@var{cmd})
## Thin gadc wrapper for GNU Octave
## See "gadc -?" for possible commands
## @end deftypefn

function ret = gadc (cmd)

  if (nargin != 1)
    print_usage ();
  endif

  [s, ret] = system ( sprintf ("gadc %s", cmd));

  if (s != 0)
    ## liballuris.h, liballuris_error
    msg = {"LIBALLURIS_MALFORMED_REPLY",
           "LIBALLURIS_DEVICE_BUSY",
           "LIBALLURIS_TIMEOUT",
           "LIBALLURIS_OUT_OF_RANGE"};
    if (s < 0)
      msg_tmp = "LIBUSB ERROR";
    elseif (s > 0 && s <= numel (msg))
      msg_tmp = msg{s};
    else
      msg_tmp = "UNKNOWN ERROR";
    endif

    error ("gadc call failed: returned %i = %s", s, msg_tmp)
  endif

  if (! isempty (ret))
    ## remove last \n
    ret(end)=[];

    ## try to convert to double
    tmp = sscanf (ret, "%f");
    if (!isempty (tmp))
      ret = tmp;
    endif
  else
    ret = [];
  endif

endfunction
