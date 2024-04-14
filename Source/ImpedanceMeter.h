/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2020 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef __IMPEDANCEMETER_H_2C4CBD67__
#define __IMPEDANCEMETER_H_2C4CBD67__

#include <DataThreadHeaders.h>

#include <stdio.h>
#include <string.h>
#include <array>
#include <atomic>

#include "rhythm-api/rhd2000ONIboard.h"
#include "rhythm-api/rhd2000registers.h"
#include "rhythm-api/rhd2000datablock.h"

#include "DeviceThread.h"

namespace ONIRhythmNode
{

	class ImpedanceMeter : public ThreadWithProgressWindow
	{
	public:

		/** Constructor*/
		ImpedanceMeter(DeviceThread* b);

		/** Destructor*/
		~ImpedanceMeter();

		/** Runs the impedance measurement*/
		void run() override;

		/** Interrupt impedance measurement thread*/
		void stopThreadSafely();
		
		/** Wait for thread to finish*/
		void waitSafely();

		/** Save values to a file (XML format)*/
		void saveValues(File& file);

		/** Select which channels to test */
		void setTestChannels(std::vector<int> channelsToTest);

	private:

		/** Calculates impedance values for all channels*/
		void runImpedanceMeasurement(Impedances& impedances);
		
		/** Restores settings of device*/
		void restoreBoardSettings();

		/** Returns the magnitude and phase (in degrees) of a selected frequency component (in Hz)
	        for a selected amplifier channel on the selected USB data stream.*/
		void measureComplexAmplitude(
			std::vector<std::vector<std::vector<double>>>& measuredMagnitude,
			std::vector<std::vector<std::vector<double>>>& measuredPhase,
			int capIndex, 
			int stream, 
			int chipChannel, 
			int numBlocks,
			double sampleRate, 
			double frequency, 
			int numPeriods);

		/** Returns the real and imaginary amplitudes of a selected frequency component in the vector
		    data, between a start index and end index. */
		void amplitudeOfFreqComponent(
			double& realComponent,					  
			double& imagComponent,					  
			const std::vector<double>& data, 				 
			int startIndex,			  
			int endIndex, 					  
			double sampleRate,   
			double frequency);

		/** Given a measured complex impedance that is the result of an electrode impedance in parallel
		    with a parasitic capacitance (i.e., due to the amplifier input capacitance and other
		    capacitances associated with the chip bondpads), this function factors out the effect of the
		    parasitic capacitance to return the acutal electrode impedance. */
		void factorOutParallelCapacitance(
			double& impedanceMagnitude,				  
			double& impedancePhase,					  
			double frequency, 					 
			double parasiticCapacitance);

		/** This is a purely empirical function to correct observed errors in the real component
		    of measured electrode impedances at sampling rates below 15 kS/s.  At low sampling rates,
		    it is difficult to approximate a smooth sine wave with the on-chip voltage DAC and 10 kHz
		    2-pole lowpass filter.  This function attempts to somewhat correct for this, but a better
		    solution is to always run impedance measurements at 20 kS/s, where they seem to be most
		    accurate. */
		void empiricalResistanceCorrection(
			double& impedanceMagnitude, 					   
			double& impedancePhase,
			double boardSampleRate);

		/** Updates electrode impedance measurement frequency, after checking that
		    requested test frequency lies within acceptable ranges based on the
		    amplifier bandwidth and the sampling rate.  See impedancefreqdialog.cpp
			for more information.*/
		float updateImpedanceFrequency(
			float desiredImpedanceFreq, 
			bool& impedanceFreqValid);

		/** Reads numBlocks blocks of raw USB data stored in a queue of Rhd2000DataBlock
            objects, loads this data into this SignalProcessor object, scaling the raw
			data to generate waveforms with units of volts or microvolts.*/
		int loadAmplifierData(
			std::queue<Rhd2000DataBlock>& dataQueue,
			int numBlocks, 
			int numDataStreams);

		std::vector<std::vector<std::vector<double>>> amplifierPreFilter;
		std::vector<bool> testChannel;

		DeviceThread* board;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ImpedanceMeter);
	};

}
#endif  // __RHD2000THREAD_H_2C4CBD67__
