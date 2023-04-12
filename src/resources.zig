const std = @import("std");

comptime {
    for (.{ "buffered", "rectangle", "sprite", "blur", "reshader" }) |shader_name| {
        const types = if (std.mem.eql(u8, shader_name, "reshader"))
            .{"vert"}
        else
            .{ "frag", "vert" };
        for (types) |shader_type| {
            const path = if (std.mem.eql(u8, shader_name, "rectangle"))
                "../resources/rect." ++ shader_type
            else
                "../resources/" ++ shader_name ++ "." ++ shader_type;

            const shader_data_pointer = @embedFile(path);
            const shader_data = shader_data_pointer.*;
            const shader_len = shader_data_pointer.len;
            @export(shader_data, .{ .name = shader_name ++ "_" ++ shader_type });
            @export(shader_len, .{ .name = shader_name ++ "_" ++ shader_type ++ "_len" });
        }
    }
}
