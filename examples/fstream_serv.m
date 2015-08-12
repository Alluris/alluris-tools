## GNU Octave script to receive the fstream output
## This creates a TCP listener on port 9000

pkg load sockets-enh
graphics_toolkit fltk

server = socket (AF_INET, SOCK_STREAM, 0);
setsockopt (server, SOL_SOCKET, SO_REUSEADDR, 1);
assert (getsockopt (server, SOL_SOCKET, SO_REUSEADDR), 1);

bind (server, 9000);
listen (server, 1);

server_data = accept (server);

# Es kommen Messwerte mit 900Hz
close all

fs = 900;
len = 4500;
data = zeros (1, len);
blk_size = 90;

## Zeitbereich
f = figure ("Position", [2 375 950 750]);
h = plot (data, "linewidth", 2);
axis ([0 len -4000 4000])
grid on

## Oberes und unteres 3*sigma Limit
std_limit = 200/3;
upper_b = line ([0 len], [0 -0], "linestyle", "--", "color", "green");
lower_b = line ([0 len], [0 -0], "linestyle", "--", "color", "green");

## Frequenzbereich
f2 = figure ("Position", [1000 375 950 750]);
ff = linspace (0, fs/2, len/2);
h2 = plot (ff, data(1:len/2), "linewidth", 2);
axis ([0 fs/2 0 1000])
xlabel ("F [Hz]");
grid on

td = zeros (50, 1);
msg_l = 0;
s = 0;
win = hanning (numel (data))';

tstart = now ();
fn = [datestr(tstart,"yyyymmdd_HHmmss"), ".log"];
fid = fopen (fn, "w");
fprintf (fid, "## Start at %s\n", datestr (tstart));

cnt =0;
unwind_protect
  tic;
  while (ishandle (f))
    if (msg_l>0)
      td = shift (td, -1);
      td(end) = msg_l./toc();
    endif
    tic ()

    [msg_s, msg_l] = recv (server_data, 4 * blk_size, MSG_WAITALL);

    v = typecast (msg_s, "int32");
    fprintf (fid, "%i\n", v);

    data = shift (data, -msg_l/4);
    data(end-(msg_l/4)+1:end) = v;
    set (h, "ydata", data);

    data_mean = mean (data);
    data_std = std (data);

    ## Update borders

    set (upper_b, "ydata", repmat (data_mean + 3 * data_std, 1, 2));
    set (lower_b, "ydata", repmat (data_mean - 3 * data_std, 1, 2));

    if (data_std > std_limit)
      set (upper_b, "color", "red");
      set (lower_b, "color", "red");
    else
      set (upper_b, "color", "green");
      set (lower_b, "color", "green");
    endif


    ## FFT
    f_data = fft (win .* data);
    f_data(1) = 0;
    f_data(2) = 0;
    fy = abs (f_data(1:len/2))/len;
    set (h2, "ydata", fy);

    drawnow ();

    [W, IW] = max (fy);
    printf ("%6.2f kB/s mean=%6.2f std=%6.2f max=%6.1f fmax=%6.2fHz\n", mean(td)/1e3, data_mean, data_std, W, ff(IW));
    fflush (stdout);

    cnt++;
    if (cnt > 10)
      cnt = 0;
      #do_html (linspace (0, len/900, len), data);
    endif

  endwhile
unwind_protect_cleanup
  tend = now ();
  fprintf (fid, "## End at %s\n", datestr (tend));
  fprintf (fid, "## Length = %is\n", (tend - tstart) * 24 * 3600);
  fclose (fid);
  ## exit fstream
  send (server_data, "c");
  disconnect (server_data);
  disconnect (server);
end
