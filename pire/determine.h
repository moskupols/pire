#ifndef PIRE_DETERMINE_H
#define PIRE_DETERMINE_H

#include "stl.h"
#include "partition.h"

namespace Pire {
	namespace Impl {

		/**
		 * An interface of a determination task.
		 * You don't have to derive from this class; it is just a start point template.
		 */
		class DetermineTask {
		private:
			struct ImplementationSpecific1;
			struct ImplementationSpecific2;

		public:
			/// A type representing a new state (may be a set of old states, a pair of them, etc...)
			typedef ImplementationSpecific1 State;

			/// A type of letter equivalence classes table.
			typedef Partition<char, ImplementationSpecific2> LettersTbl;

			/// A container used for storing map of states to thier indices.
			typedef ymap<State, size_t> InvStates;

			/// Should return used letters' partition.
			const LettersTbl& Letters() const;

			/// Should return initial state (surprise!)
			State Initial() const;

			/// Should calculate next state, given the current state and a letter.
			State Next(State state, Char letter) const;

			/// Should return true iff the state need to be processed.
			bool IsRequired(const State& /*state*/) const { return true; }

			/// Called when the set of new states is closed.
			void AcceptStates(const yvector<State>& newstates);

			/// Called for each transition from one new state to another.
			void Connect(size_t from, size_t to, Char letter);

			typedef bool Result;
			Result Success() { return true; }
			Result Failure() { return false; }
		};

		/**
		 * A helper function for FSM determining and all determine-like algorithms
		 * like scanners' agglutination.
		 *
		 * Given an indirectly specified automaton (through Task::Initial() and Task::Next()
		 * functions, see above), performs a breadth-first traversal, finding and enumerating
		 * all effectively reachable states. Then passes all found states and transitions
		 * between them back to the task.
		 *
		 * Initial state is always placed at zero position.
		 *
		 * Please note that the function does not take care of any payload (including final flags);
		 * it is the task's responsibility to agglutinate them properly.
		 *
		 * Returns task.Succeed() if everything was done; task.Failure() if maximum limit of state count was reached.
		 */
		template<class Task>
		typename Task::Result Determine(Task& task, size_t maxSize)
		{
			typedef typename Task::State State;
			typedef typename Task::LettersTbl Letters;
			typedef typename Task::InvStates InvStates;
			typedef ydeque< yvector<size_t> > TransitionTable;

			yvector<State> states;
			InvStates invstates;
			TransitionTable transitions;
			yvector<size_t> stateIndices;

			states.push_back(task.Initial());
			invstates.insert(typename InvStates::value_type(states[0], 0));

			for (size_t stateIdx = 0; stateIdx < states.size(); ++stateIdx) {
				if (!task.IsRequired(states[stateIdx]))
					continue;
				TransitionTable::value_type row(task.Letters().Size());
				for (typename Letters::ConstIterator lit = task.Letters().Begin(), lie = task.Letters().End(); lit != lie; ++lit) {
					State newState = task.Next(states[stateIdx], lit->first);
					typename InvStates::const_iterator i = invstates.find(newState);
					if (i == invstates.end()) {
						if (!maxSize--)
							return task.Failure();
						i = invstates.insert(typename InvStates::value_type(newState, states.size())).first;
						states.push_back(newState);
					}
					row[lit->second.first] = i->second;
				}
				transitions.push_back(row);
				stateIndices.push_back(stateIdx);
			}

			yvector<Char> invletters(task.Letters().Size());
			for (typename Letters::ConstIterator lit = task.Letters().Begin(), lie = task.Letters().End(); lit != lie; ++lit)
				invletters[lit->second.first] = lit->first;

			task.AcceptStates(states);
			size_t from = 0;
			for (TransitionTable::iterator i = transitions.begin(), ie = transitions.end(); i != ie; ++i, ++from) {
				yvector<Char>::iterator l = invletters.begin();
				for (TransitionTable::value_type::iterator j = i->begin(), je = i->end(); j != je; ++j, ++l)
					task.Connect(stateIndices[from], *j, *l);
			}
			return task.Success();
		}
	}
}

#endif
