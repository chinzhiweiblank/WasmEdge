//! Defines WasmEdge Table and TableType structs.
//!
//! A WasmEdge `Table` defines a table described by its `TableType`.
//! `TableType` specifies the limits on the size of a table. The start of
//! the limit range specifies the lower bound (inclusive) of the size, while
//! the end resticts the upper bound (inclusive).

use crate::{
    error::{check, TableError, WasmEdgeError},
    ffi,
    types::{RefType, WasmValue},
    WasmEdgeResult,
};
use std::ops::RangeInclusive;

/// Struct of WasmEdge Table.
///
/// A WasmEdge [Table] defines a table described by its [TableType].
#[derive(Debug)]
pub struct Table {
    pub(crate) inner: InnerTable,
    pub(crate) registered: bool,
}
impl Table {
    /// Creates a new [Table] to be associated with the given element type and the size.
    ///
    /// # Arguments
    ///
    /// - `ty` specifies the type of the new [Table].
    ///
    /// # Error
    ///
    /// If fail to create a [Table], then an error is returned.
    ///
    /// # Example
    ///
    /// ```
    /// use wasmedge_sys::{RefType, TableType, Table};
    /// // create a TableType instance
    /// let ty = TableType::create(RefType::FuncRef, 10..=20).expect("fail to create a TableType");
    ///
    /// // create a Table instance
    /// let table = Table::create(&ty).expect("fail to create a Table");
    /// ```
    pub fn create(ty: &TableType) -> WasmEdgeResult<Self> {
        let ctx = unsafe { ffi::WasmEdge_TableInstanceCreate(ty.inner.0) };

        match ctx.is_null() {
            true => Err(WasmEdgeError::Table(TableError::Create)),
            false => Ok(Table {
                inner: InnerTable(ctx),
                registered: false,
            }),
        }
    }

    /// Returns the [TableType] of the [Table].
    ///
    /// # Error
    ///
    /// If fail to get type, then an error is returned.
    pub fn ty(&self) -> WasmEdgeResult<TableType> {
        let ty_ctx = unsafe { ffi::WasmEdge_TableInstanceGetTableType(self.inner.0) };
        match ty_ctx.is_null() {
            true => Err(WasmEdgeError::Table(TableError::Type)),
            false => Ok(TableType {
                inner: InnerTableType(ty_ctx as *mut _),
                registered: true,
            }),
        }
    }

    /// Returns the element value at a specific position in the [Table].
    ///
    /// # Arguments
    ///
    /// - `idx` specifies the position in the [Table], at which the [WasmValue](crate::WasmValue) is returned.
    ///
    /// # Error
    ///
    /// If fail to get the data, then an error is returned.
    pub fn get_data(&self, idx: u32) -> WasmEdgeResult<WasmValue> {
        let raw_val = unsafe {
            let mut data = ffi::WasmEdge_ValueGenI32(0);
            check(ffi::WasmEdge_TableInstanceGetData(
                self.inner.0,
                &mut data as *mut _,
                idx,
            ))?;
            data
        };
        Ok(raw_val.into())
    }

    /// Sets a new element value at a specific position in the [Table].
    ///
    /// # Arguments
    ///
    /// - `data` specifies the new data.
    ///
    /// - `idx` specifies the position of the new data to be stored in the [Table].
    ///
    /// # Error
    ///
    /// If fail to set data, then an error is returned.
    pub fn set_data(&mut self, data: WasmValue, idx: u32) -> WasmEdgeResult<()> {
        unsafe {
            check(ffi::WasmEdge_TableInstanceSetData(
                self.inner.0,
                data.as_raw(),
                idx,
            ))
        }
    }

    /// Returns the capacity of the [Table].
    ///
    /// # Example
    ///
    /// ```
    /// use wasmedge_sys::{RefType, TableType, Table};
    ///
    /// // create a TableType instance and a Table
    /// let ty = TableType::create(RefType::FuncRef, 10..=20).expect("fail to create a TableType");
    /// let table = Table::create(&ty).expect("fail to create a Table");
    ///
    /// // check capacity
    /// assert_eq!(table.capacity(), 10);
    /// ```
    ///
    pub fn capacity(&self) -> usize {
        unsafe { ffi::WasmEdge_TableInstanceGetSize(self.inner.0) as usize }
    }

    /// Increases the capacity of the [Table].
    ///
    /// After growing, the new capacity must be in the range defined by `limit` when the table is created.
    ///
    /// # Argument
    ///
    /// - `size` specifies the size to be added to the [Table].
    ///
    /// # Error
    ///
    /// If fail to increase the size of the [Table], then an error is returned.
    pub fn grow(&mut self, size: u32) -> WasmEdgeResult<()> {
        unsafe { check(ffi::WasmEdge_TableInstanceGrow(self.inner.0, size)) }
    }
}
impl Drop for Table {
    fn drop(&mut self) {
        if !self.registered && !self.inner.0.is_null() {
            unsafe {
                ffi::WasmEdge_TableInstanceDelete(self.inner.0);
            }
        }
    }
}

#[derive(Debug)]
pub(crate) struct InnerTable(pub(crate) *mut ffi::WasmEdge_TableInstanceContext);
unsafe impl Send for InnerTable {}
unsafe impl Sync for InnerTable {}

/// Struct of WasmEdge TableType
///
/// A WasmEdge [TableType] classify a [Table] over elements of element types within a size range.
#[derive(Debug)]
pub struct TableType {
    pub(crate) inner: InnerTableType,
    pub(crate) registered: bool,
}
impl Drop for TableType {
    fn drop(&mut self) {
        if !self.registered && !self.inner.0.is_null() {
            unsafe {
                ffi::WasmEdge_TableTypeDelete(self.inner.0);
            }
        }
    }
}
impl TableType {
    /// Creates a new [TableType] to be associated with the given limit range of the size and the reference type.
    ///
    /// # Arguments
    ///
    /// - `elem_type` specifies the element type.
    ///
    /// - `limit` specifies a range of the table size. The upper bound for a `limit` is `u32::MAX`.
    ///
    /// # Error
    ///
    /// If fail to create a [TableType], then an error is returned.
    ///
    /// # Example
    ///
    /// ```ignore
    /// let ty = TableType::create(RefType::FuncRef, 10..=20).expect("fail to create a TableType");
    /// ```
    ///
    pub fn create(elem_ty: RefType, limit: RangeInclusive<u32>) -> WasmEdgeResult<Self> {
        let ctx = unsafe {
            ffi::WasmEdge_TableTypeCreate(
                ffi::WasmEdge_RefType::from(elem_ty),
                ffi::WasmEdge_Limit::from(limit),
            )
        };
        match ctx.is_null() {
            true => Err(WasmEdgeError::TableTypeCreate),
            false => Ok(Self {
                inner: InnerTableType(ctx),
                registered: false,
            }),
        }
    }

    /// Returns the element type.
    pub fn elem_ty(&self) -> RefType {
        let ty = unsafe { ffi::WasmEdge_TableTypeGetRefType(self.inner.0) };
        ty.into()
    }

    /// Returns a range of the limit size of a [Table].
    ///
    /// # Example
    ///
    /// ```
    /// use wasmedge_sys::{RefType, TableType};
    ///
    /// // create a TableType instance
    /// let ty = TableType::create(RefType::FuncRef, 10..=20).expect("fail to create a TableType");
    ///
    /// // check limit
    /// assert_eq!(ty.limit(), 10..=20);
    /// ```
    pub fn limit(&self) -> RangeInclusive<u32> {
        let limit = unsafe { ffi::WasmEdge_TableTypeGetLimit(self.inner.0) };
        limit.into()
    }
}

#[derive(Debug)]
pub(crate) struct InnerTableType(pub(crate) *mut ffi::WasmEdge_TableTypeContext);
unsafe impl Send for InnerTableType {}
unsafe impl Sync for InnerTableType {}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::{FuncType, Function, RefType, ValType};
    use std::{
        sync::{Arc, Mutex},
        thread,
    };

    #[test]
    fn test_table_type() {
        // create a TableType instance
        let result = TableType::create(RefType::FuncRef, 10..=20);
        assert!(result.is_ok());
        let ty = result.unwrap();
        assert!(!ty.inner.0.is_null());
        assert!(!ty.registered);

        // check element type
        assert_eq!(ty.elem_ty(), RefType::FuncRef);
        // check limit
        assert_eq!(ty.limit(), 10..=20);
    }

    #[test]
    fn test_table() {
        // create a TableType instance
        let result = TableType::create(RefType::FuncRef, 10..=20);
        assert!(result.is_ok());
        let ty = result.unwrap();

        // create a Table instance
        let result = Table::create(&ty);
        assert!(result.is_ok());
        let mut table = result.unwrap();

        // check capacity
        assert_eq!(table.capacity(), 10);

        // get type
        let result = table.ty();
        assert!(result.is_ok());
        let ty = result.unwrap();
        assert!(!ty.inner.0.is_null());
        assert!(ty.registered);

        // check limit and element type
        assert_eq!(ty.limit(), 10..=20);
        assert_eq!(ty.elem_ty(), RefType::FuncRef);

        // grow the capacity of table
        let result = table.grow(5);
        assert!(result.is_ok());
        // check capacity
        assert_eq!(table.capacity(), 15);
    }

    #[test]
    fn test_table_data() {
        // create a FuncType
        let result = FuncType::create(vec![ValType::I32; 2], vec![ValType::I32]);
        assert!(result.is_ok());
        let func_ty = result.unwrap();
        // create a host function
        let result = Function::create(&func_ty, Box::new(real_add), 0);
        assert!(result.is_ok());
        let mut host_func = result.unwrap();

        // create a TableType instance
        let result = TableType::create(RefType::FuncRef, 10..=20);
        assert!(result.is_ok());
        let ty = result.unwrap();

        // create a Table instance
        let result = Table::create(&ty);
        assert!(result.is_ok());
        let mut table = result.unwrap();

        // check capacity
        assert_eq!(table.capacity(), 10);

        // get data in the scope of the capacity
        let result = table.get_data(9);
        assert!(result.is_ok());
        let value = result.unwrap();
        assert!(value.is_null_ref());
        assert_eq!(value.ty(), ValType::FuncRef);

        // set data
        let result = table.set_data(WasmValue::from_func_ref(&mut host_func), 3);
        assert!(result.is_ok());
        // get data
        let result = table.get_data(3);
        assert!(result.is_ok());
        let value = result.unwrap();
        let result = value.func_ref();
        assert!(result.is_some());
        let func_ref = result.unwrap();

        let result = func_ref.ty();
        assert!(result.is_ok());
        let func_ty = result.unwrap();
        assert_eq!(func_ty.params_len(), 2);
        let param_tys = func_ty.params_type_iter().collect::<Vec<_>>();
        assert_eq!(param_tys, [ValType::I32, ValType::I32]);
        assert_eq!(func_ty.returns_len(), 1);
        let return_tys = func_ty.returns_type_iter().collect::<Vec<_>>();
        assert_eq!(return_tys, [ValType::I32]);
    }

    #[test]
    fn test_table_send() {
        // create a TableType instance
        let result = TableType::create(RefType::FuncRef, 10..=20);
        assert!(result.is_ok());
        let ty = result.unwrap();

        // create a Table instance
        let result = Table::create(&ty);
        assert!(result.is_ok());
        let table = result.unwrap();

        let handle = thread::spawn(move || {
            assert!(!table.inner.0.is_null());

            // check capacity
            assert_eq!(table.capacity(), 10);

            // get type
            let result = table.ty();
            assert!(result.is_ok());
            let ty = result.unwrap();
            assert!(!ty.inner.0.is_null());
            assert!(ty.registered);

            // check limit and element type
            assert_eq!(ty.limit(), 10..=20);
            assert_eq!(ty.elem_ty(), RefType::FuncRef);
        });

        handle.join().unwrap();
    }

    #[test]
    fn test_table_sync() {
        // create a TableType instance
        let result = TableType::create(RefType::FuncRef, 10..=20);
        assert!(result.is_ok());
        let ty = result.unwrap();

        // create a Table instance
        let result = Table::create(&ty);
        assert!(result.is_ok());
        let table = Arc::new(Mutex::new(result.unwrap()));

        let table_cloned = Arc::clone(&table);
        let handle = thread::spawn(move || {
            let result = table_cloned.lock();
            assert!(result.is_ok());
            let table = result.unwrap();

            // check capacity
            assert_eq!(table.capacity(), 10);

            // get type
            let result = table.ty();
            assert!(result.is_ok());
            let ty = result.unwrap();
            assert!(!ty.inner.0.is_null());
            assert!(ty.registered);

            // check limit and element type
            assert_eq!(ty.limit(), 10..=20);
            assert_eq!(ty.elem_ty(), RefType::FuncRef);
        });

        handle.join().unwrap();
    }

    fn real_add(input: Vec<WasmValue>) -> Result<Vec<WasmValue>, u8> {
        println!("Rust: Entering Rust function real_add");

        if input.len() != 2 {
            return Err(1);
        }

        let a = if input[0].ty() == ValType::I32 {
            input[0].to_i32()
        } else {
            return Err(2);
        };

        let b = if input[1].ty() == ValType::I32 {
            input[0].to_i32()
        } else {
            return Err(3);
        };

        let c = a + b;
        println!("Rust: calcuating in real_add c: {:?}", c);

        println!("Rust: Leaving Rust function real_add");
        Ok(vec![WasmValue::from_i32(c)])
    }
}
