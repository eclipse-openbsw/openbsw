import sys
import bt2
from bt2 import trace_class as bt2_trace_class
sys.path.append(".")
from trace_convert import Event, TraceParser

bt2.register_plugin(__name__, "openbsw")

class OpenBSWSourceIter(bt2._UserMessageIterator):
    def __init__(self, config, output_port):
        print("OpenBSWSourceIter: Creating for port {}".format(output_port))

        self._input, self._trace_class = output_port.user_data

        self._stream_class = self._trace_class[0]
        self._stream = self._trace_class().create_stream(self._stream_class)

        self._event_class = []
        for i in range(len(self._stream_class)):
            self._event_class.append(self._stream_class[i])

        self._events = TraceParser(self._input).read()
        self._index = 0
        self._count = len(self._events)

    def __next__(self):
        if self._count > 0 and self._index < self._count:
            e = self._events[self._index]
            event_msg = self._create_event_message(
                self._event_class[e.id],
                self._stream,
                default_clock_snapshot=e.timestamp
            )
            event_msg.event.payload_field["id"] = e.arg
            self._index += 1
            return event_msg
        else:
            raise StopIteration

@bt2.plugin_component_class
class OpenBSWSource(bt2._UserSourceComponent, message_iterator_class=OpenBSWSourceIter):
    def __init__(self, config, params, obj):
        print("OpenBSWSource: Creating with params {}".format(params))

        if "inputs" not in params:
            raise ValueError("OpenBSWSource: missing `inputs` parameter")

        inputs = params["inputs"]

        if type(inputs) != bt2._ArrayValueConst:
            raise TypeError(
                "OpenBSWSource: expecting `inputs` parameter to be a list, got a {}".format(
                    type(inputs)
                )
            )

        if len(inputs) != 1:
            raise ValueError(
                "OpenBSWSource: expecting `inputs` parameter to be of length, got {}".format(
                    len(inputs)
                )
            )

        if type(inputs[0]) != bt2._StringValueConst:
            raise TypeError(
                "OpenBSWSource: expecting `inputs[0]` parameter to be a string, got a {}".format(
                    type(inputs[0])
                )
            )

        trace_class = self._create_metadata()

        self._add_output_port("out", (str(inputs[0]), trace_class))

    def _create_event_class(self, trace_class, event_id, name, arg) -> None:
        stream_class = trace_class[0]
        payload_class = trace_class.create_structure_field_class()
        payload_class.append_member(arg, trace_class.create_unsigned_integer_field_class())
        stream_class.create_event_class(id=event_id, name=name, payload_field_class=payload_class)

    def _create_metadata(self) -> bt2_trace_class._TraceClass:
        trace_class = self._create_trace_class()
        clock_class = self._create_clock_class()
        stream_class = trace_class.create_stream_class(
            assigns_automatic_event_class_id=False,
            default_clock_class=clock_class
        )

        self._create_event_class(trace_class, 0, "thread_switched_out", "id")
        self._create_event_class(trace_class, 1, "thread_switched_in", "id")
        self._create_event_class(trace_class, 2, "isr_enter", "id")
        self._create_event_class(trace_class, 3, "isr_exit", "id")
        self._create_event_class(trace_class, 4, "user", "id")

        return trace_class
