/*********************************************************************
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2009, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Willow Garage nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *********************************************************************/

// Author: Blaise Gassend
#ifndef __DIAGNOSTIC_UPDATER__DRIVER_H__
#define __DIAGNOSTIC_UPDATER__DRIVER_H__

#include <ros/publisher.h>
#include <ros/subscription.h>
#include <diagnostic_updater/update_functions.h>

namespace diagnostic_updater
{
     
/**
 * \brief A class to facilitate making diagnostics for a topic using a
 * FrequencyStatus. 
 *
 * The word "headerless" in the class name refers to the fact that it is
 * mainly designed for use with messages that do not have a header, and
 * that cannot therefore be checked using a TimeStampStatus.
 */

class HeaderlessTopicDiagnostic : public CombinationDiagnosticTask
{
public:
/**
 * \brief Constructs a HeaderlessTopicDiagnostic. 
 *
 * \param name The name of the topic that is being diagnosed.
 *
 * \param diag The diagnostic_updater that the CombinationDiagnosticTask
 * should add itself to.
 *
 * \param freq The parameters for the FrequencyStatus class that will be
 * computing statistics.
 */
	
	HeaderlessTopicDiagnostic(
      std::string name,
      diagnostic_updater::Updater &diag,
      const diagnostic_updater::FrequencyStatusParam &freq) :
    CombinationDiagnosticTask(name + " topic status"), 
    freq_(freq)
  {
    addTask(&freq_);
    diag.add(*this);
  }

  virtual ~HeaderlessTopicDiagnostic()
  {}
  
  /**
	 * \brief Signals that a publication has occurred.
	 */

  virtual void tick()
  {
    freq_.tick();
  }
  
  /**
	 * \brief Clears the frequency statistics.
	 */

  virtual void clear_window()
  {
    freq_.clear();
  }

private:
  diagnostic_updater::FrequencyStatus freq_;
};

/**
 * \brief A HeaderlessTopicDiagnostic combined with a ros::Publisher.
 *
 * For a standard ros::Publisher, this class allows the ros::Publisher and
 * the HeaderlessTopicDiagnostic to be combined for added convenience.
 */

class HeaderlessDiagnosedPublisher : public HeaderlessTopicDiagnostic
{
public:
  /**
   * \brief Constructs a HeaderlessTopicDiagnostic. 
   *
   * \param pub The publisher on which statistics are being collected.
   *
   * \param diag The diagnostic_updater that the CombinationDiagnosticTask
   * should add itself to.
   *
   * \param freq The parameters for the FrequencyStatus class that will be
   * computing statistics.
   */

  HeaderlessDiagnosedPublisher(const ros::Publisher &pub,
      diagnostic_updater::Updater &diag,
      const diagnostic_updater::FrequencyStatusParam &freq) :
    HeaderlessTopicDiagnostic(pub.getTopic(), diag, freq),
    publisher_(pub)
  {}

  virtual ~HeaderlessDiagnosedPublisher()
  {}
  
  /**
	 * \brief Collects statistics and publishes the message.
	 */
  virtual void publish(const ros::MessageConstPtr& message)
  {
    tick();
    publisher_.publish(message);
  }
 
  /**
	 * \brief Collects statistics and publishes the message.
	 */
  virtual void publish(const ros::Message& message)
  {
    tick();
    publisher_.publish(message);
  }

  /**
	 * \brief Returns the publisher.
	 */
  ros::Publisher getPublisher() const
  {
    return publisher_;
  }

  /**
	 * Deprecated, use getPublisher instead.
	 */
  
	ROSCPP_DEPRECATED ros::Publisher publisher() const
  {
    return publisher_;
  }

  /**
	 * \brief Changes the publisher.
	 */
  void setPublisher(ros::Publisher pub)
  {
    publisher_ = pub;
  }

  /**
	 * Deprecated, use setPublisher instead.
	 */

  ROSCPP_DEPRECATED void set_publisher(ros::Publisher pub)
  {
    publisher_ = pub;
  }

private:
  ros::Publisher publisher_;
};

/**
 * \brief A class to facilitate making diagnostics for a topic using a
 * FrequencyStatus and TimeStampStatus. 
 */

class TopicDiagnostic : public HeaderlessTopicDiagnostic
{
public:
/**
 * \brief Constructs a TopicDiagnostic. 
 *
 * \param name The name of the topic that is being diagnosed.
 *
 * \param diag The diagnostic_updater that the CombinationDiagnosticTask
 * should add itself to.
 *
 * \param freq The parameters for the FrequencyStatus class that will be
 * computing statistics.
 *
 * \param stamp The parameters for the TimeStampStatus class that will be
 * computing statistics.
 */
	
  TopicDiagnostic(
      std::string name,
      diagnostic_updater::Updater &diag,
      const diagnostic_updater::FrequencyStatusParam &freq,
      const diagnostic_updater::TimeStampStatusParam &stamp) : 
    HeaderlessTopicDiagnostic(name, diag, freq), 
    stamp_(stamp)
  {
    addTask(&stamp_);
  }
  
  virtual ~TopicDiagnostic()
  {}
  
  /**
	 * This method should never be called on a TopicDiagnostic as a timestamp
	 * is needed to collect the timestamp diagnostics. It is defined here to
	 * prevent the inherited tick method from being used accidentally.
	 */
	virtual void tick() { ROS_FATAL("tick(void) has been called on a TopicDiagnostic. This is never correct. Use tick(ros::Time &) instead."); }

  /**
	 * \brief Collects statistics and publishes the message.
	 *
	 * \param stamp Timestamp to use for interval computation by the
	 * TimeStampStatus class.
	 */
  virtual void tick(const ros::Time &stamp)
  {
    stamp_.tick(stamp);
    HeaderlessTopicDiagnostic::tick();
  }
  
private:
  TimeStampStatus stamp_;
};

/**
 * \brief A TopicDiagnostic combined with a ros::Publisher.
 *
 * For a standard ros::Publisher, this class allows the ros::Publisher and
 * the TopicDiagnostic to be combined for added convenience.
 */

template<class T>
class DiagnosedPublisher : public TopicDiagnostic
{
public:
/**
 * \brief Constructs a DiagnosedPublisher. 
 *
 * \param pub The publisher on which statistics are being collected.
 *
 * \param diag The diagnostic_updater that the CombinationDiagnosticTask
 * should add itself to.
 *
 * \param freq The parameters for the FrequencyStatus class that will be
 * computing statistics.
 *
 * \param stamp The parameters for the TimeStampStatus class that will be
 * computing statistics.
 */
	
  DiagnosedPublisher(const ros::Publisher &pub,
      diagnostic_updater::Updater &diag, 
      const diagnostic_updater::FrequencyStatusParam &freq, 
      const diagnostic_updater::TimeStampStatusParam &stamp) : 
    TopicDiagnostic(pub.getTopic(), diag, freq, stamp),
    publisher_(pub)
  {}

  virtual ~DiagnosedPublisher()
  {}
  
  /**
	 * \brief Collects statistics and publishes the message.
	 *
	 * The timestamp to be used by the TimeStampStatus class will be
	 * extracted from message.header.stamp.
	 */
	virtual void publish(const boost::shared_ptr<T>& message) {
		tick(message->header.stamp); publisher_.publish(message); }
 
  /**
	 * \brief Collects statistics and publishes the message.
	 *
	 * The timestamp to be used by the TimeStampStatus class will be
	 * extracted from message.header.stamp.
	 */
	virtual void publish(const T& message) { tick(message.header.stamp);
		publisher_.publish(message); }

  /**
	 * \brief Returns the publisher.
	 */
  ros::Publisher getPublisher() const
  {
    return publisher_;
  }

  /**
	 * Deprecated, use getPublisher instead.
	 */
  
	ROSCPP_DEPRECATED ros::Publisher publisher() const
  {
    return publisher_;
  }

  /**
	 * \brief Changes the publisher.
	 */
  void setPublisher(ros::Publisher pub)
  {
    publisher_ = pub;
  }

  /**
	 * Deprecated, use setPublisher instead.
	 */

  ROSCPP_DEPRECATED void set_publisher(ros::Publisher pub)
  {
    publisher_ = pub;
  }

private:
  ros::Publisher publisher_;
};

};

#endif
