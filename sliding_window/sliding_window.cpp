#include "sliding_window.hpp"
#include <stdio.h>

namespace sw
{
using namespace packet;
using namespace std;

SlidingWindow::SlidingWindow(int size, int bufferSize)
{
	printf("BUFFER : %d\n", bufferSize);
	this->bufferSize = bufferSize;
	this->size = size;
	this->start = 0;
	this->end = this->start + size;
	this->availableFrame = size;
	this->acks.resize(size);
	this->frames.resize(this->bufferSize);
	this->locked = false;
	this->dumped = this->start;

	this->windowForwardCallback = [](vector<Frame> &) {};
}

SlidingWindow::SlidingWindow(const SlidingWindow &window)
{
	this->bufferSize = window.bufferSize;
	this->size = window.getSize();
	this->start = window.getStart();
	this->end = window.getEnd();
	this->availableFrame = this->size;
	this->acks.resize(this->size);
	this->frames.resize(this->bufferSize);
	this->locked = false;
	this->dumped = this->start;

	this->windowForwardCallback = [](vector<Frame> &) {};
}

bool SlidingWindow::addFrame(Frame frame)
{
	if (this->locked) {
		printf("LOCKED\n");
		return false;
	}

	if (frame.getSeqNum() - this->dumped >= this->frames.size())
	{
		printf("SIZE NOT ENOUGH\n");
		return false;
	}

	if (frame.getSeqNum() > this->end)
	{
		printf("OUT OF BOUND\n");
		return false;
	}

	if (this->availableFrame == 0 && this->frames[frame.getSeqNum() - this->dumped].getSOH() == 0x0)
	{
		printf("WINDOW SIZE : %d\n", this->size);
		printf("NO AVAIL FRAME\n");
		return false;
	}

	if (this->frames[frame.getSeqNum() - this->dumped].getSOH() == 0x0) {
		this->availableFrame--;
	}

	this->frames[frame.getSeqNum() - this->dumped] = frame;
	return true;
}

bool SlidingWindow::addACK(ACK ack)
{
	if (this->locked) {
		return false;
	}

	if (ack.getAck() != 0x6)
	{
		printf("NOT ACK\n");
		return false;
	}

	if (ack.getNextSeqNum() > this->acks.size())
	{
		printf("ACK SIZE NOT ENOUGH\n");
		return false;
	}

	if (ack.getNextSeqNum() - 1 > this->end)
	{
		printf("ACK OUT OF BOUND\n");
		return false;
	}

	this->acks[ack.getNextSeqNum() - 1] = ack;
	while (this->acks[this->start].getAck() == 0x6)
	{
		printf("MAJUUUUUUU\n");
		this->start++;
		this->end++;
		this->acks.resize(this->acks.size() + 1);
		//this->frames.resize(this->frames.size() + 1);
		this->availableFrame++;
	}
	int count = 0;
	for (int i = this->dumped; i < this->dumped + this->bufferSize; i++) {
		if (this->acks[i].getAck() == 0x6) {
			count++;
		} else {
			break;
		}
	}

	if (count == this->bufferSize) {
		this->windowForwardCallback(this->frames);
		this->dumped += this->bufferSize;
	}


	return true;
}

Frame &SlidingWindow::getFrame(int seqNum)
{
	if (this->frames.size() <= seqNum - this->dumped)
	{
		return this->frames[0];
	}

	return this->frames[seqNum - this->dumped];
}

ACK &SlidingWindow::getACK(int seqNum)
{
	if (this->acks.size() <= seqNum)
	{
		return this->acks[0];
	}

	return this->acks[seqNum];
}

void SlidingWindow::setSize(int size)
{
	this->size = size;
}

void SlidingWindow::setStart(int start)
{
	this->start = start;
}

void SlidingWindow::setEnd(int end)
{
	this->end = end;
}

void SlidingWindow::setWFCallback(std::function<void(vector<Frame> &)> func)
{
	this->windowForwardCallback = func;
}

void SlidingWindow::incrementAvailableFrame()
{
	this->availableFrame++;
}

void SlidingWindow::setAvailableFrame(int availableFrame) {
	this->availableFrame = availableFrame;
}

int SlidingWindow::getSize() const
{
	return this->size;
}

int SlidingWindow::getStart() const
{
	return this->start;
}

int SlidingWindow::getEnd() const
{
	return this->end;
}

void SlidingWindow::decrementAvailableFrame()
{
	this->availableFrame--;
}
} // namespace sw