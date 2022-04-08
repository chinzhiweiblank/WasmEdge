(module
  (func $i (import "extern" "func-parse-json") (param i32) (result i32) )
  (func (export "parse-json") (param i32) (result i32)
    local.get 0
    call $i
  )
)