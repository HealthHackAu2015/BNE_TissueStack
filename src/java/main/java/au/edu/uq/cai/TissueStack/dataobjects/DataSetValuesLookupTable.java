/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
package au.edu.uq.cai.TissueStack.dataobjects;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.GenerationType;
import javax.persistence.Id;
import javax.persistence.JoinColumn;
import javax.persistence.OneToOne;
import javax.persistence.Table;
import javax.xml.bind.annotation.XmlElement;
import javax.xml.bind.annotation.XmlRootElement;
import javax.xml.bind.annotation.XmlTransient;

@Entity
@Table(name="dataset_values_lookup")
@XmlRootElement(name="DataSetValuesLookupTable", namespace=IGlobalConstants.XML_NAMESPACE)
public class DataSetValuesLookupTable {
	private long id;
	private String filename; 
	private String content;
	private AtlasInfo associatedAtlas;

	@Id
	@Column(name="id")
	@GeneratedValue(strategy = GenerationType.IDENTITY)
	@XmlTransient
	public long getId() {
		return id;
	}
	public void setId(long id) {
		this.id = id;
	}
	
	@Column(name="filename")
	@XmlElement(name="FileName", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getFilename() {
		return filename;
	}
	
	public void setFilename(String filename) {
		this.filename = filename;
	}
	
	@Column(name="content")
	@XmlElement(name="Content", namespace=IGlobalConstants.XML_NAMESPACE)
	public String getContent() {
		return this.content;
	}
	
	public void setContent(String content) {
		this.content = content;
	} 
	
	@OneToOne
	@JoinColumn(name="atlas_association")
	@XmlElement(name="AssociatedAtlas", namespace=IGlobalConstants.XML_NAMESPACE)
	public AtlasInfo getAssociatedAtlas() {
		return this.associatedAtlas;
	}
	public void setAssociatedAtlas(AtlasInfo associatedAtlas) {
		this.associatedAtlas = associatedAtlas;
	}
	
	public boolean equals(Object obj) {
		if (!(obj instanceof DataSetValuesLookupTable)) return false;
		
		final DataSetValuesLookupTable castObj = (DataSetValuesLookupTable) obj;
		if (this.id == castObj.id) return true;
		
		return false;
	}
	
	public int hashCode() {
		return new Long(this.id).intValue();
	}
}
