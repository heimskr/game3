#pragma once

#include "ui/gl/Types.h"

namespace Game3 {
	class HasAlignment {
		public:
			HasAlignment(Alignment vertical, Alignment horizontal);
			HasAlignment();

			virtual Alignment getVerticalAlignment() const;
			virtual Alignment getHorizontalAlignment() const;
			virtual void setVerticalAlignment(Alignment);
			virtual void setHorizontalAlignment(Alignment);

		protected:
			Alignment verticalAlignment = Alignment::Start;
			Alignment horizontalAlignment = Alignment::Start;
	};
}
