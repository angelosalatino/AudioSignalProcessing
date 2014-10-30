%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% HCI
%
% Audio Signal Processing using MATLAB
%
%
%                          Angelo Antonio Salatino
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%% Reading file
% Section ID = 1

% data = sample collection
% fs = sampling frequency

filename = './test.wav';
[data,fs] = wavread(filename);
%or
%[data,fs] = audioread(filename);
filename_copy = './testCopy.wav';
audiowrite(filename_copy,data,fs)

clear filename

%% Information & play
% Section ID = 2

% tempo is track length (in seconds)
numberOfSamples = length(data);
tempo = numberOfSamples / fs;

disp (sprintf('Length: %f seconds',tempo));
disp (sprintf('Number of Samples %d', numberOfSamples));
disp (sprintf('Sampling Frequency %d Hz',fs));
disp (sprintf('Number of Channels: %d', min(size(data))));

%play file
sound(data,fs);
%PLOT the signal
time = linspace(0,tempo,numberOfSamples);
plot(time,data);

clear tempo

%% Conversion from 2 Channels to 1
% Section ID = 3

if min(size(data))>1 %in case there are more than 1 channel
    A = mean(data');
    clear data
    data=A';
    clear A
else
    disp(sprintf('The signal has only one channel'));
end

%% Framing
% Section ID = 4

timeWindow = 0.04; % Frame length in term of seconds. Default: timeWindow = 40ms
timeStep = 0.01; % seconds between two frames. Default: timeStep = 10ms (in case of OVERLAPPING)

overlap = 1; % 1 in case of overlap, 0 no overlap


sampleForWindow = timeWindow * fs;

if overlap == 0;
    Y = buffer(data,sampleForWindow);
else
    sampleToJump = sampleForWindow - timeStep * fs;
    Y = buffer(data,sampleForWindow,ceil(sampleToJump));
end

[m,n]=size(Y); % m corresponds to sampleForWindow
numFrames = n;

disp(sprintf('Number of Frames: %d',numFrames));

clear sampleToJump m n overlap


%% Windowing
% Section ID = 5

num_points = sampleForWindow;

% some windows USE help window
w_gauss =  gausswin(num_points);
w_hamming =  hamming(num_points);
w_hann =  hann(num_points); 
plot(1:num_points,[w_gauss,w_hamming, w_hann]); axis([1 num_points 0 2]);
legend('Gaussian','Hamming','Hann');

old_Y = Y;

for i=1:numFrames
    Y(:,i)=Y(:,i).*w_hann;
end

%see the difference

index_to_plot = 88;
figure
plot (old_Y(:,index_to_plot))
hold on
plot (Y(:,index_to_plot), 'green')
hold off


clear num_points w_gauss w_hamming w_hann

%% Energy
% Section ID = 6

% It requires that signal is already framed
% Run Section ID=4

for i=1:numFrames
    energy(i)=sum(abs(Y(:,i)).^2); % Windowed Signal
    %energy(i)=sum(abs(old_Y(:,i)).^2);
end

figure, plot(energy)
xlabel('# of frames')

%% Fast Fourier Transform (sull'intero segnale)
% Section ID = 7

NFFT = 2^nextpow2(numberOfSamples); % Next higher power of 2. (in order to optimize FFT computation)
freqSignal = fft(data,NFFT);
f = fs/2*linspace(0,1,NFFT/2+1);

% PLOT the magnitude of fft of data
plot(f,abs(freqSignal(1:NFFT/2+1)))
title('Single-Sided Amplitude Spectrum of y(t)')
xlabel('Frequency (Hz)')
ylabel('|Y(f)|')

clear NFFT freqSignal f

%% Short Term Fourier Transform
% Section ID = 8

% It requires that signal is already framed and windowed
% Run Section ID = 4 and 5

NFFT = 2^nextpow2(sampleForWindow); 
STFT = ones(NFFT,numFrames);

for i=1:numFrames
    STFT(:,i)=fft(Y(:,i),NFFT);
end

indexToPlot = 80; %frame index to plot


if indexToPlot < numFrames
    f = fs/2*linspace(0,1,NFFT/2+1);
    
    % PLOT
    plot(f,2*abs(STFT(1:NFFT/2+1,indexToPlot)))
    title(sprintf('FFT del frame %d', indexToPlot));
    xlabel('Frequency (Hz)')
    ylabel(sprintf('|STFT_{%d}(f)|',indexToPlot))
else
    disp('Unable to create plot');
end

specgram(data,sampleForWindow,fs)
title('Spectrogram [dB]')

clear indexToPlot

%% Auto-Correlation per frames
% Section ID = 9
 
% It requires that signal is already framed
% Run Section ID=4

for i=1:numFrames
    autoCorr(:,i)=xcorr(Y(:,i));
end

indexToPlot = 80; %frame index to plot

if indexToPlot < numFrames
  
    % PLOT
    plot(autoCorr(sampleForWindow:end,i))

else
    disp('Unable to create plot');
end

clear indexToPlot

