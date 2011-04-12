function plotdiffeph(file,sats)

if nargin<1, file='diffeph.out'; end
if nargin<2, sats=1:32; end

v=textread(file);

figure('color','w'), hold on, box on, grid on

clk0=v(v(:,3)==2,5); % reference clock = PRN2

for sat=sats
    i=find(v(:,3)==sat);
    if isempty(i), continue; end
    time=v(i,2)-floor(v(1,2)/86400)*86400;
    poserr=v(i,4:6);
    clkerr=v(i,5)-clk0;
    
    plot(time/3600,[poserr,clkerr],'.-');
    
    disp(sprintf('PRN%02d: %4d  %9.4f %9.4f %9.4f %9.4f m',sat,length(time),...
         sqrt(mean(poserr.^2,1)),sqrt(mean(clkerr.^2,1))));
end
xlim(time([1,end])/3600);
ylim([-0.5,0.5]);
xlabel('Time (hr)');
ylabel('Error (m)');
title('Ephemeris Error');
legend({'Radial','Along-Trk','Cross-Trk','Clock'});
moveax;

