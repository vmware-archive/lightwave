using System;
namespace VMDir.Common.DTO
{
	public class GroupDTO
	{
		public string cn { get; set; }
		public int groupType { get; set; }
		public string sAMAccountName { get; set; }
		public string objectClass { get; set; }
	}
}

