#pragma once

#include "data/Identifier.h"
#include "net/Buffer.h"
#include "ui/gl/widget/Widget.h"

#include <any>
#include <string>

namespace Game3 {
	class Agent;

	class Module: public Widget {
		public:
			Module();

			virtual ~Module() = default;

			virtual Identifier getID() const = 0;
			virtual void reset();
			virtual void update();
			virtual std::optional<Buffer> handleMessage(const std::shared_ptr<Agent> &, const std::string &, std::any &);
	};
}
